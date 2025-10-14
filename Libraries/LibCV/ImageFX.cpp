#include "ImageFX.h"
#include "opencv2/core.hpp"
#include "opencv2/photo.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

namespace LibCV
{
	std::shared_ptr<Image> ImageFX::AdjustBrightnessContrast(const std::shared_ptr<Image>& image, double clipHistPercent)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};

		cv::Mat gray;
		cv::cvtColor(*(cv::Mat*)image->cvMatPtr, gray, cv::COLOR_BGR2GRAY);

		// Compute histogram
		int histSize = 256;
		float range[] = { 0, 256 };
		const float* histRange = { range };
		cv::Mat hist;
		cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

		// Cumulative distribution
		std::vector<float> accumulator(histSize);
		accumulator[0] = hist.at<float>(0);
		for (int i = 1; i < histSize; i++)
			accumulator[i] = accumulator[i - 1] + hist.at<float>(i);

		float max = accumulator.back();
		clipHistPercent *= (max / 100.0);
		clipHistPercent /= 2.0;

		// Locate points to clip
		int minGray = 0;
		while (accumulator[minGray] < clipHistPercent)
			minGray++;

		int maxGray = histSize - 1;
		while (accumulator[maxGray] >= (max - clipHistPercent))
			maxGray--;

		double alpha = 255.0 / (maxGray - minGray);
		double beta = -minGray * alpha;

		cv::Mat result;
		((cv::Mat*)image->cvMatPtr)->convertTo(*(cv::Mat*)results->cvMatPtr, -1, alpha, beta);

		return results;
	}

	std::shared_ptr<Image> ImageFX::AdjustGamma(const std::shared_ptr<Image>& image, double gamma)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::Mat lut(1, 256, CV_8UC1);
		uchar* p = lut.ptr();
		for (int i = 0; i < 256; i++)
			p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, 1.0 / gamma) * 255.0);
		cv::LUT(*(cv::Mat*)image->cvMatPtr, lut, *(cv::Mat*)results->cvMatPtr);
		return results;
	}

	std::shared_ptr<Image> ImageFX::AdjustColorTemperature(const std::shared_ptr<Image>& image, int kelvinShift)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};

		cv::Mat lab;
		cv::cvtColor(*((cv::Mat*)image->cvMatPtr), lab, cv::COLOR_BGR2Lab);

		std::vector<cv::Mat> lab_channels(3);
		cv::split(lab, lab_channels);

		// Convert to float to prevent overflow/underflow
		lab_channels[2].convertTo(lab_channels[2], CV_32F);
		lab_channels[2] += kelvinShift;
		lab_channels[2] = cv::min(cv::max(lab_channels[2], 0.0f), 255.0f);
		lab_channels[2].convertTo(lab_channels[2], CV_8U);

		cv::Mat merged;
		cv::merge(lab_channels, merged);

		cvtColor(merged, *(cv::Mat*)results->cvMatPtr, cv::COLOR_Lab2BGR);

		return results;
	}

	std::shared_ptr<Image> ImageFX::ApplyCLAHE(const std::shared_ptr<Image>& image)
	{
		cv::Mat lab;
		cv::cvtColor(*((cv::Mat*)image->cvMatPtr), lab, cv::COLOR_BGR2Lab);

		std::vector<cv::Mat> lab_channels(3);
		cv::split(lab, lab_channels);

		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size{ 8, 8 });
		clahe->apply(lab_channels[0], lab_channels[0]);

		cv::Mat merged;
		cv::merge(lab_channels, merged);

		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cvtColor(merged, *(cv::Mat*)results->cvMatPtr, cv::COLOR_Lab2BGR);
		return results;
	}

	std::shared_ptr<Image> ImageFX::ApplyEnhanceDetails(const std::shared_ptr<Image>& image)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::detailEnhance(
			*((cv::Mat*)image->cvMatPtr), 
			*((cv::Mat*)results->cvMatPtr),
			20.0, 
			0.3f);
		return results;
	}

	std::shared_ptr<Image> ImageFX::ApplyPencilSketch(const std::shared_ptr<Image>& image, bool gray)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };

		cv::Mat graySketch, coloredSketch;
		cv::pencilSketch(
			*((cv::Mat*)image->cvMatPtr),
			graySketch,
			coloredSketch);

		if (gray)
		{
			cv::cvtColor(graySketch, graySketch, cv::COLOR_GRAY2BGR);
			results->cvMatPtr = new cv::Mat{ graySketch };
		}
		else
			results->cvMatPtr = new cv::Mat{ coloredSketch };

		return results;
	}

	std::shared_ptr<Image> ImageFX::ApplyDenoise(const std::shared_ptr<Image>& image)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::fastNlMeansDenoisingColored(
			*((cv::Mat*)image->cvMatPtr),
			*((cv::Mat*)results->cvMatPtr),
			10, 
			10,
			7, 
			21);
		return results;
	}

	std::shared_ptr<Image> ImageFX::AutoEnhance(const std::shared_ptr<Image>& image, unsigned flags)
	{
		auto results = image;
		
		if(flags & AUTO_BRIGHTNESS_CONTRAST)
			results = AdjustBrightnessContrast(results, 1.0);

		if (flags & AUTO_GAMMA)
		{
			cv::Mat gray;
			cv::cvtColor(*(cv::Mat*)results->cvMatPtr, gray, cv::COLOR_BGR2GRAY);
			double meanBrightness = mean(gray)[0];
			double gamma = 1.0;
			if (meanBrightness < 90)
				gamma = 1.2;
			else if (meanBrightness > 180)
				gamma = 0.8;

			results = AdjustGamma(results, gamma);
		}

		if (flags & AUTO_COLOR_TEMP)
		{
			cv::Scalar meanBGR = cv::mean(*(cv::Mat*)results->cvMatPtr);
			double blueRatio = meanBGR[0] / (meanBGR[2] + 1e-5);
			int kelvinShift = 0;
			if (blueRatio > 1.05)
				kelvinShift = 10;   // warmer
			else if (blueRatio < 0.95)
				kelvinShift = -10;  // cooler
			results = AdjustColorTemperature(results, kelvinShift);
		}
		
		results = flags & AUTO_CLAHE ? ApplyCLAHE(results) : results;
		
		results = flags & AUTO_DETAIL_ENHANCE ? ApplyEnhanceDetails(results) : results;
		
		results = flags & AUTO_DENOISE ? ApplyDenoise(results) : results;

		return results;
	}
}

