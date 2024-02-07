#include <opencv2/opencv.hpp>
#include <iostream>

const int SIZE = 10;

int main() {
    // Путь к файлу изображения; настройте его в соответствии с вашим окружением
    std::string imagePath = "../source/2024-02-05 13.47.36.jpg";

    // Загрузка изображения
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

    if(image.empty()) // Проверка на неудачную загрузку
    {
        std::cout << "Could not open or find the image" << std::endl;
        std::cin.get(); // Ожидание нажатия клавиши
        return -1;
    }

    // Переворачиваем изображение
    cv::Mat flippedImage;
    cv::flip(image, flippedImage, 0);

    // Инвертируем цвета изображения
    cv::Mat invertedImage;
    cv::bitwise_not(flippedImage, invertedImage);

    // Создание шахматной маски того же размера, что и изображение
    cv::Mat mask = cv::Mat::zeros(invertedImage.rows, invertedImage.cols, CV_8UC1);
    for(int y = 0; y < mask.rows; ++y) {
        for(int x = 0; x < mask.cols; ++x) {
            if((x / SIZE) % 2 == (y / SIZE) % 2) { // Изменение размера клетки можно настроить через делитель
                mask.at<uchar>(y, x) = 255;
            }
        }
    }

    // Преобразуем маску из одного канала в три канала для соответствия изображению
    cv::Mat maskColor;
    cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);

    // Применяем маску через побитовое И к инвертированному изображению
    cv::Mat result;
    cv::bitwise_and(invertedImage, maskColor, result);

    // Создаём окно для отображения
    cv::namedWindow("Chessboard Masked Image", cv::WINDOW_NORMAL);

    // Показываем результат в окне
    cv::imshow("Chessboard Masked Image", result);

    // Ожидание нажатия клавиши
    cv::waitKey(0);

    return 0;
}
