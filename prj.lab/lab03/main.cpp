#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>

// Функция для автоконтрастирования одного канала
void autoContrastChannel(cv::Mat& channel, double q_b, double q_w) {
    // Преобразование типа для работы с гистограммой
    channel.convertTo(channel, CV_32F);

    // Вычисление квантилей
    std::vector<float> pixels;
    pixels.assign((float*)channel.datastart, (float*)channel.dataend);
    std::sort(pixels.begin(), pixels.end());

    float lowerBound = pixels[(int)(q_b * pixels.size())];
    float upperBound = pixels[(int)(q_w * pixels.size())];

    // Применение автоконтрастирования
    for (int i = 0; i < channel.rows; ++i) {
        for (int j = 0; j < channel.cols; ++j) {
            float& pixel = channel.at<float>(i, j);
            if (pixel < lowerBound) pixel = 0;
            else if (pixel > upperBound) pixel = 255;
            else pixel = (pixel - lowerBound) / (upperBound - lowerBound) * 255;
        }
    }

    channel.convertTo(channel, CV_8U); // Возвращение к исходному типу
}

// Функция для отрисовки гистограммы яркости для одного канала
cv::Mat draw_histogram(const cv::Mat& src) {
    std::vector<cv::Mat> bgr_planes;
    cv::split(src, bgr_planes);

    int histSize = 256;
    float range[] = { 0, 256 };
    const float* histRange = { range };
    bool uniform = true, accumulate = false;
    std::vector<cv::Mat> histArray;

    // Отрисовка гистограммы для каждого канала
    for (int i = 0; i < src.channels(); ++i) {
        cv::Mat hist;
        cv::calcHist(&bgr_planes[i], 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);

        int hist_w = 512; int hist_h = 400;
        int bin_w = cvRound((double) hist_w / histSize);

        cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(0,0,0));

        // Нормализация результатов к [0, histImage.rows]
        cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

        // Отрисовка для каждого бина
        for (int j = 1; j < histSize; j++) {
            cv::line(histImage, cv::Point(bin_w*(j-1), hist_h - cvRound(hist.at<float>(j-1))),
                     cv::Point(bin_w*(j), hist_h - cvRound(hist.at<float>(j))),
                     cv::Scalar(255, 0, 0), 2, 8, 0);
        }

        histArray.push_back(histImage);
    }

    // Возвращаем только первую гистограмму для примера
    // В вашем случае, вы можете выбрать, как отображать все гистограммы
    return histArray[0];
}

int main(int argc, char** argv) {

    double q_b = 0.1, q_w = 0.1; // значения по умолчанию
    std::string inputFilename = "../source/x.jpeg"; // значение по умолчанию

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-q_b" && i + 1 < argc) {
            q_b = std::stod(argv[++i]);
        } else if (arg == "-q_w" && i + 1 < argc) {
            q_w = std::stod(argv[++i]);
        } else {
            inputFilename = arg;
        }
    }
    std::cout << q_b << " " << q_w << "\n";

    // Чтение изображения
    cv::Mat image = cv::imread(inputFilename);
    if (image.empty()) {
        std::cout << "Could not open or find the image" << std::endl;
        return -1;
    }

    // Если ваше изображение цветное, измените количество каналов на 3
    const int Number_of_channels = 3; // Для цветного изображения
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    // Параметры квантилей для автоконтрастирования
    q_w = 1-q_w; // Верхний квантиль

    std::vector<cv::Mat> histograms;

    // Применение автоконтрастирования и отрисовка гистограмм для каждого канала
    for (int i = 0; i < Number_of_channels; ++i) {
        autoContrastChannel(channels[i], q_b, q_w);
        histograms.push_back(draw_histogram(channels[i]));
    }

    // Слияние обработанных каналов обратно в одно изображение и показ гистограмм
    cv::Mat result;
    cv::merge(channels, result);

    cv::imshow("Histogram", histograms[0]);
    cv::imshow("Auto-Contrasted Image", result);
    cv::waitKey(0);

    cv::imwrite("auto_contrasted_image.png", result);

    return 0;
}
