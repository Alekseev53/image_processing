#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

// Функция для гамма-коррекции трехканального изображения
cv::Mat applyGammaCorrection(const cv::Mat& img, double gamma) {
    CV_Assert(img.depth() == CV_8U); // Проверяем, что глубина изображения равна 8 бит на канал

    // Создаем LUT для гамма-коррекции
    cv::Mat lut(1, 256, CV_8UC1);
    for (int i = 0; i < 256; ++i) {
        lut.at<uchar>(i) = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
    }

    std::vector<cv::Mat> channels;
    cv::split(img, channels);

    // Применяем LUT только к красному каналу
    cv::LUT(channels[2], lut, channels[2]);

    cv::Mat result;
    cv::merge(channels, result);

    return result;
}

int main(int argc, char** argv) {
    // Аргументы командной строки и значения по умолчанию
    int s = 50, h = 100; // значения по умолчанию
    double gamma = 2.4;
    std::string outputFilename = "output.png"; // значение по умолчанию

    // Парсинг аргументов командной строки
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-s" && i + 1 < argc) {
            s = std::stoi(argv[++i]);
        } else if (arg == "-h" && i + 1 < argc) {
            h = std::stoi(argv[++i]);
        } else if (arg == "-gamma" && i + 1 < argc) {
            gamma = std::stod(argv[++i]);
        } else {
            outputFilename = arg; // Предполагаем, что не ключевой аргумент — это имя файла
        }
    }

    // Создание градиентного изображения от темно-красного до светло-красного
    int rows = 256 * h / s;
    cv::Mat img(rows, s, CV_8UC3); // Используем трехканальный тип изображения

    for (int i = 0; i < img.rows; ++i) {
        uchar value = static_cast<uchar>((i * 255.0) / (rows - 1));
        cv::Vec3b color(value, 0, 0); // Темно-красный до светло-красного: изменяем только красный канал
        for (int j = 0; j < img.cols; ++j) {
            img.at<cv::Vec3b>(i, j) = color;
        }
    }

    // Применение гамма-коррекции
    cv::Mat correctedImg = applyGammaCorrection(img, gamma);

    // Вывод изображения на экран или сохранение в файл
    if (outputFilename.empty()) {
        cv::imshow("Gradient with Gamma Correction", correctedImg);
        cv::waitKey(0);
    } else {
        cv::imwrite(outputFilename, correctedImg);
    }

    return 0;
}
