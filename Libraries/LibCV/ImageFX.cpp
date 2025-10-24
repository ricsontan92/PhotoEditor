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

	std::shared_ptr<Image> ImageFX::AdjustSharpen(const std::shared_ptr<Image>& image)
	{
		cv::Mat gray;
		cv::cvtColor(*(cv::Mat*)image->cvMatPtr, gray, cv::COLOR_BGR2GRAY);

		// Compute Laplacian variance (measure of sharpness)
		cv::Mat lap;
		cv::Laplacian(gray, lap, CV_64F);
		cv::Scalar meanLap, stdLap;
		cv::meanStdDev(lap, meanLap, stdLap);
		double variance = stdLap[0] * stdLap[0];

		// Map variance → sharpening strength automatically
		// Lower variance → stronger sharpening
		double strength;
		if (variance < 50.0)
			strength = 2.0; // very blurry
		else if (variance < 150.0)
			strength = 1.0 + (150.0 - variance) / 100.0; // mild blur
		else if (variance < 300.0)
			strength = 0.5;
		else
			strength = 0.0; // already sharp, skip

		// Apply sharpening only if needed
		if (strength > 0.05)
		{
			std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
			results->cvMatPtr = new cv::Mat{};

			cv::Mat blurred;
			cv::GaussianBlur(*(cv::Mat*)image->cvMatPtr, blurred, cv::Size(0, 0), 2.0);
			cv::addWeighted(*(cv::Mat*)image->cvMatPtr, 1.0 + strength, blurred, -strength, 0, *(cv::Mat*)results->cvMatPtr);
			return results;
		}

		// Return unchanged if already sharp
		return image->Clone();
	}

	std::shared_ptr<Image> ImageFX::AdjustHSL(const std::shared_ptr<Image>& image)
	{
		cv::Mat hls;
		cv::cvtColor(*(cv::Mat*)image->cvMatPtr, hls, cv::COLOR_BGR2HLS);

		std::vector<cv::Mat> channels;
		cv::split(hls, channels);

		cv::Mat H = channels[0]; // Hue
		cv::Mat L = channels[1]; // Lightness
		cv::Mat S = channels[2]; // Saturation

		// --- Compute mean lightness and saturation ---
		cv::Scalar meanL = cv::mean(L);
		cv::Scalar meanS = cv::mean(S);

		double meanLight = meanL[0];
		double meanSat = meanS[0];

		// --- Auto Lightness Adjustment ---
		// Target ~128 (midpoint)
		double lightScale = 1.0;
		if (meanLight < 100)
			lightScale = 1.0 + (128 - meanLight) / 256.0;
		else if (meanLight > 160)
			lightScale = 1.0 - (meanLight - 128) / 256.0;

		L.convertTo(L, CV_32F);
		L = cv::min(cv::max(L * lightScale, 0.0f), 255.0f);
		L.convertTo(L, CV_8U);

		// --- Auto Saturation Adjustment ---
		// Boost low-saturation images, dampen oversaturated ones
		double satScale = 1.0;
		if (meanSat < 90)
			satScale = 1.3;
		else if (meanSat < 128)
			satScale = 1.1;
		else if (meanSat > 200)
			satScale = 0.8;

		S.convertTo(S, CV_32F);
		S = cv::min(cv::max(S * satScale, 0.0f), 255.0f);
		S.convertTo(S, CV_8U);

		// --- Optional: Small Hue correction (neutralize color cast) ---
		// Compute average hue bias toward warm/cool tones
		cv::Scalar meanHue = cv::mean(H);
		double hueShift = 0.0;
		if (meanHue[0] < 30 || meanHue[0] > 150) hueShift = 2.0;   // cool → slightly warm
		else if (meanHue[0] > 90 && meanHue[0] < 150) hueShift = -2.0; // warm → slightly cool

		H.convertTo(H, CV_32F);
		H += hueShift;
		H = cv::min(cv::max(H, 0.0f), 180.0f);
		H.convertTo(H, CV_8U);

		// --- Merge and convert back ---
		cv::merge(std::vector< cv::Mat>{ H, L, S }, hls);
		
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::cvtColor(hls, *(cv::Mat*)results->cvMatPtr, cv::COLOR_HLS2BGR);

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

		results = flags & AUTO_SHARPEN ? AdjustSharpen(results) : results;

		results = flags & AUTO_HSL ? AdjustHSL(results) : results;


		return results;
	}
}

