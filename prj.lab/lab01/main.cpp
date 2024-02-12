#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

// Функция для гамма-коррекции трехканального изображения
cv::Mat applyGammaCorrection(const cv::Mat& img, double gamma) {
    CV_Assert(img.depth() == CV_8U); // Проверяем, что глубина изображения равна 8 бит на канал

    cv::Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; ++i) {
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
    }

    cv::Mat result;
    cv::LUT(img, lut, result); // Применяем LUT к изображению

    return result;
}

int main(int argc, char** argv) {
    int s = 50, h = 100; // значения по умолчанию
    double gamma = 2.4;
    std::string outputFilename = "output.png"; // значение по умолчанию

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            s = std::stoi(argv[++i]);
        } else if (arg == "-h" && i + 1 < argc) {
            h = std::stoi(argv[++i]);
        } else if (arg == "-gamma" && i + 1 < argc) {
            gamma = std::stod(argv[++i]);
        } else {
            outputFilename = arg;
        }
    }

    int rows = 256 * h / s;
    cv::Mat img(rows, s * 2, CV_8UC3); // Создаем изображение в два раза шире

    // Создание градиентной полосы без гамма-коррекции
    for (int i = 0; i < img.rows; ++i) {
        uchar value = static_cast<uchar>((i * 255.0) / (rows - 1));
        cv::Vec3b color(value, 0, 0);
        for (int j = 0; j < img.cols / 2; ++j) { // Заполняем только первую половину ширины
            img.at<cv::Vec3b>(i, j) = color;
        }
    }

    // Создание градиентной полосы с гамма-коррекцией
    cv::Mat halfImg = img.colRange(0, s);
    cv::Mat correctedHalfImg = applyGammaCorrection(halfImg, gamma);
    correctedHalfImg.copyTo(img.colRange(s, s * 2)); // Копируем обработанную половину во вторую половину ширины исходного изображения

     cv::Mat rotatedImg;
    cv::rotate(img, rotatedImg, cv::ROTATE_90_CLOCKWISE);

    // Сохранение или отображение результата
    if (!outputFilename.empty()) {
        cv::imwrite(outputFilename, rotatedImg);
    } else {
        cv::imshow("Gradient Original and Gamma Corrected Rotated", rotatedImg);
        cv::waitKey(0);
    }

    return 0;
}
