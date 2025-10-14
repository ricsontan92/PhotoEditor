#include "Image.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

namespace LibCV
{
	std::shared_ptr<Image> Image::Create(const std::filesystem::path& path)
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{ cv::imread(path.string()) };
		return results;
	}

	void Image::Show(const char* windowName, float resize) const
	{
		cv::Mat clone = resize != 1.0f ? ((cv::Mat*)cvMatPtr)->clone() : *((cv::Mat*)cvMatPtr);

		if (resize != 1.0f)
			cv::resize(clone, clone, cv::Size{0, 0}, resize, resize);

		cv::imshow(windowName, clone);
		cv::waitKey(0);
		cv::destroyWindow(windowName);
	}

	void Image::Write(const char* filename) const
	{
		std::vector<int> params;
		params.push_back(cv::IMWRITE_JPEG_QUALITY);
		params.push_back(100);  // 100 = maximum quality, minimum compression

		cv::imwrite(filename, *((cv::Mat*)cvMatPtr), params);
	}

	std::shared_ptr<Image> Image::AdjustGamma(double gamma) const
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::Mat lut(1, 256, CV_8UC1);
		uchar* p = lut.ptr();
		for (int i = 0; i < 256; i++)
			p[i] = cv::saturate_cast<uchar>(pow(i / 255.0, 1.0 / gamma) * 255.0);
		cv::LUT(*(cv::Mat*)cvMatPtr, lut, *(cv::Mat*)results->cvMatPtr);
		return results;
	}

	std::shared_ptr<Image> Image::AutoBrightnessContrast(double clipHistPercent, double& alpha, double& beta) const
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};

		cv::Mat gray;
		cv::cvtColor(*(cv::Mat*)cvMatPtr, gray, cv::COLOR_BGR2GRAY);

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

		alpha = 255.0 / (maxGray - minGray);
		beta = -minGray * alpha;

		cv::Mat result;
		((cv::Mat*)cvMatPtr)->convertTo(*(cv::Mat*)results->cvMatPtr, -1, alpha, beta);

		return results;
	}

	std::shared_ptr<Image> Image::AdjustColorTemperature(int kelvinShift) const
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};

		cv::Mat lab;
		cv::cvtColor(*((cv::Mat*)cvMatPtr), lab, cv::COLOR_BGR2Lab);

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

	std::shared_ptr<Image> Image::AutoEnhance() const
	{
		double alpha, beta;
		auto image = AutoBrightnessContrast(1.0, alpha, beta);

		// --- Estimate brightness and apply gamma correction ---
		cv::Mat gray;
		cv::cvtColor(*(cv::Mat*)image->cvMatPtr, gray, cv::COLOR_BGR2GRAY);
		double meanBrightness = mean(gray)[0];
		double gamma = 1.0;
		if (meanBrightness < 90)
			gamma = 1.2;
		else if (meanBrightness > 180)
			gamma = 0.8;

		image = image->AdjustGamma(gamma);

		// --- Estimate color balance to decide warm/cool shift ---
		cv::Scalar meanBGR = cv::mean(*(cv::Mat*)image->cvMatPtr);
		double blueRatio = meanBGR[0] / (meanBGR[2] + 1e-5);
		int kelvinShift = 0;
		if (blueRatio > 1.05)
			kelvinShift = 10;   // warmer
		else if (blueRatio < 0.95)
			kelvinShift = -10;  // cooler

		return image->AdjustColorTemperature(kelvinShift);
	}

	std::shared_ptr<Image> Image::Resize(float percentage) const
	{
		auto ht = ((cv::Mat*)cvMatPtr)->rows;
		auto wd = ((cv::Mat*)cvMatPtr)->cols;
		return Resize(static_cast<unsigned>(roundf(wd * percentage)), static_cast<unsigned>(std::roundf(ht * percentage)));
	}

	std::shared_ptr<Image> Image::Resize(unsigned width, unsigned height) const
	{
		std::shared_ptr<Image> results = std::shared_ptr<Image>{ new Image{} };
		results->cvMatPtr = new cv::Mat{};
		cv::resize(
			*((cv::Mat*)cvMatPtr),
			*((cv::Mat*)results->cvMatPtr),
			cv::Size{ static_cast<int>(width), static_cast<int>(height) }
		);
		return results;
	}

	unsigned Image::Width() const
	{
		return ((cv::Mat*)cvMatPtr)->cols;
	}

	unsigned Image::Height() const
	{
		return ((cv::Mat*)cvMatPtr)->rows;
	}

	unsigned Image::Channels() const
	{
		return ((cv::Mat*)cvMatPtr)->channels();
	}

	LibCore::Math::Vec4 Image::Pixel(unsigned x, unsigned y) const
	{
		LibCore::Math::Vec4 results;
		switch (((cv::Mat*)cvMatPtr)->depth())
		{
		case CV_8U:
		{
			const float scale = 1.0f / 255.0f;

			switch (Channels())
			{
			case 1:
			{
				uchar v = ((cv::Mat*)cvMatPtr)->at<uchar>(y, x);
				results = LibCore::Math::Vec4(v, v, v, 255.0f) * scale;
			} break;
			case 3:
			{
				cv::Vec3b pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec3b>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], 255) * scale; // BGR → RGBW
			} break;
			case 4:
			{
				cv::Vec4b pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec4b>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], pix[3]) * scale; // BGRW → RGBW
			} break;
			}
			break;
		}

		case CV_16U:
		{
			const float scale = 1.0f / 65535.0f;

			switch (Channels())
			{
			case 1:
			{
				uint16_t v = ((cv::Mat*)cvMatPtr)->at<uint16_t>(y, x);
				results = LibCore::Math::Vec4(v, v, v, 65535) * scale;
			} break;
			case 3:
			{
				cv::Vec3w pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec3w>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], 65535) * scale; // BGR → RGBW
			} break;
			case 4:
			{
				cv::Vec4w pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec4w>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], pix[3]) * scale; // BGRW → RGBW
			} break;
			}
			break;
		}

		case CV_32F:
		{
			switch (Channels())
			{
			case 1:
			{
				float v = ((cv::Mat*)cvMatPtr)->at<float>(y, x);
				results = LibCore::Math::Vec4(v, v, v, 1.0f);
			} break;
			case 3:
			{
				cv::Vec3f pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec3f>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], 1.0f); // BGR → RGBW
			} break;
			case 4:
			{
				cv::Vec4f pix = ((cv::Mat*)cvMatPtr)->at<cv::Vec4f>(y, x);
				results = LibCore::Math::Vec4(pix[2], pix[1], pix[0], pix[3]); // BGRW → RGBW
			} break;
			}
			break;
		}

		default:
			throw std::runtime_error("Unsupported image depth");
		}

		return results;
	}

	ImageData Image::GetImageData() const
	{
		ImageData results;
		results.ImageWidth = ((cv::Mat*)cvMatPtr)->cols;
		results.ImageHeight = ((cv::Mat*)cvMatPtr)->rows;
		results.ImageChannels = ((cv::Mat*)cvMatPtr)->channels();
		results.Pixels = std::vector<char>{ ((cv::Mat*)cvMatPtr)->datastart, ((cv::Mat*)cvMatPtr)->dataend };
		return results;
	}

	Image::Image()
		: cvMatPtr{ nullptr }
	{

	}

	Image::~Image()
	{
		delete (cv::Mat*)cvMatPtr;
	}
}