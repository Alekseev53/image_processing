#include <opencv2/opencv.hpp>
#include <vector>
#include <random>
#include <numeric>

// Функция для вычисления среднего значения вектора
double mean_of_vector(const std::vector<float>& values) {
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

// Функция для вычисления дисперсии вектора
double variance_of_vector(const std::vector<float>& values, double mean) {
    double variance = 0.0;
    for (auto value : values) {
        variance += (value - mean) * (value - mean);
    }
    variance /= values.size();
    return variance;
}

// Функция для вычисления среднего индекса вектора
double mean_index_of_vector(const std::vector<float>& values) {
    double sum = 0.0;
    double weight_sum = 0.0;
    for (size_t i = 0; i < values.size(); ++i) {
        sum += i * values[i];
        weight_sum += values[i];
    }
    return weight_sum > 0 ? sum / weight_sum : 0;
}



// Function to generate the test image with three levels of brightness
cv::Mat generate_test_image(int side, int inner_square_side, int circle_radius, const std::vector<uchar>& levels) {
    cv::Mat image = cv::Mat::zeros(side, side, CV_8UC1);

    // Outer square
    image.setTo(levels[0]);

    // Inner square
    cv::Point inner_square_top_left((side - inner_square_side) / 2, (side - inner_square_side) / 2);
    cv::rectangle(image, inner_square_top_left, inner_square_top_left + cv::Point(inner_square_side, inner_square_side), levels[1], cv::FILLED);

    // Circle
    cv::circle(image, cv::Point(side / 2, side / 2), circle_radius, levels[2], cv::FILLED);

    return image;
}


// Function to draw the histogram of brightness
cv::Mat draw_histogram(const cv::Mat& src) {
    int histSize = 256; // Количество бинов
    float range[] = { 0, 256 }; // Диапазон значений
    const float* histRange = { range };
    cv::Mat hist;
    cv::calcHist(&src, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

    // Преобразование cv::Mat гистограммы в std::vector<float>
    std::vector<float> hist_values(histSize);
    for (int i = 0; i < histSize; ++i) {
        hist_values[i] = hist.at<float>(i);
    }

    // Размеры изображения гистограммы
    int hist_w = 256;
    int hist_h = 256;
    cv::Mat histImage(hist_h, hist_w, CV_8UC1, cv::Scalar(230));

    // Нахождение максимального значения в гистограмме
    double maxVal = 0;
    cv::minMaxLoc(hist, 0, &maxVal);

    // Нормализация гистограммы так, чтобы максимальное значение соответствовало 230 пикселям
    hist = hist * (230.0 / maxVal);

    // Отрисовка гистограммы
    for(int i = 1; i < histSize; i++) {
        cv::line(histImage, 
                 cv::Point(i, hist_h), 
                 cv::Point(i, hist_h - cvRound(hist.at<float>(i))), 
                 cv::Scalar(0), 
                 1);
    }

    // Разбиваем гистограмму на три части и вычисляем средний индекс для каждой части
    size_t part_size = histSize / 3;
    for (int part = 0; part < 3; ++part) {
        auto start_iter = hist_values.begin() + part * part_size;
        auto end_iter = (part < 2) ? start_iter + part_size : hist_values.end();
        std::vector<float> part_values(start_iter, end_iter);
        double mean_index = mean_index_of_vector(part_values);

        // Отрисовка линии среднего индекса для каждой части
        int line_position = cvRound(mean_index + part * part_size);
        cv::line(histImage, cv::Point(line_position, 0), cv::Point(line_position, hist_h), cv::Scalar(0), 2);
    }

    return histImage;
}

// Function to add noise to the image
cv::Mat add_noise(const cv::Mat& src, double stddev) {
    cv::Mat noise = cv::Mat(src.size(), CV_32F);  // Use floating-point precision for noise
    std::normal_distribution<float> dist(0, stddev);
    std::default_random_engine generator;

    // Generate noise
    for (int i = 0; i < noise.rows; i++) {
        for (int j = 0; j < noise.cols; j++) {
            noise.at<float>(i, j) = dist(generator);
        }
    }

    // Add noise to the original image
    cv::Mat noisy_image;
    src.convertTo(noisy_image, CV_32F);  // Convert src to float for addition
    noisy_image += noise;  // Add the noise

    // Clip the values to [0, 255] and convert back to uchar
    cv::Mat clipped_noisy_image;
    cv::threshold(noisy_image, clipped_noisy_image, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(clipped_noisy_image, clipped_noisy_image, 0, 0, cv::THRESH_TOZERO);
    clipped_noisy_image.convertTo(clipped_noisy_image, CV_8U);

    return clipped_noisy_image;
}

// Assuming you want to store histogram values and some other integer data together
void extractHistogramValues(std::vector<std::pair<std::vector<float>, std::vector<int>>>& histValuesVec,cv::Mat& src, std::vector<int> relatedData) {
    int histSize = 256; // Number of bins
    float range[] = { 0, 256 }; // Range of values
    const float* histRange = { range };
    cv::Mat hist;
    cv::calcHist(&src, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, true, false);

    std::vector<float> histValues(histSize);

    for (int i = 0; i < histSize; ++i) {
        histValues[i] = hist.at<float>(i);
        // Populate relatedData[i] as needed
    }

    // Store the pair of vectors in the provided vector of pairs
    histValuesVec.push_back(std::make_pair(histValues, relatedData));
}

int main() {
    int side = 256;
    int inner_square_side = 209;
    int circle_radius = 83;

    // Brightness levels for test images
    std::vector<std::vector<uchar>> levels = {
        {0, 127, 255},
        {20, 127, 235},
        {55, 127, 200},
        {90, 127, 165}
    };

    // Standard deviation values for noise
    std::vector<double> stddev_values = {3, 7, 15};

    // Generate test images and histograms
    std::vector<cv::Mat> test_images;
    for(const auto& level : levels) {
        cv::Mat image = generate_test_image(side, inner_square_side, circle_radius, level);
        test_images.push_back(image);
    }

    // Concatenate test images horizontally
    cv::Mat test_image_row;
    cv::hconcat(test_images, test_image_row);

    // Generate noisy images and their histograms
    std::vector<cv::Mat> all_noisy_images_with_histograms;
    for(double stddev : stddev_values) {
        std::vector<cv::Mat> noisy_images_with_histograms;
        for(auto& image : test_images) {
            cv::Mat noisy_image = add_noise(image, stddev);
            cv::Mat histogram = draw_histogram(noisy_image); // Отрисовка гистограммы со средним индексом
            cv::Mat combined_image;
            cv::vconcat(noisy_image, histogram, combined_image);
            noisy_images_with_histograms.push_back(combined_image);
        }

        // Concatenate noisy images with histograms for the current stddev horizontally
        cv::Mat noisy_images_row;
        cv::hconcat(noisy_images_with_histograms, noisy_images_row);
        all_noisy_images_with_histograms.push_back(noisy_images_row);
    }

    // Concatenate all noisy images with histograms vertically with the test images row
    cv::Mat final_image = test_image_row;
    for (auto& noisy_row : all_noisy_images_with_histograms) {
        cv::vconcat(final_image, noisy_row, final_image);
    }

    // Vector to store histogram values for all images
    std::vector<std::pair<std::vector<float>,std::vector<int>>> all_histogram_values;
    // Process and store histograms for original and noisy images
    int i_k = 0;
    for (auto& image : test_images) {
        //(levels[i][0]+levels[i][1])/2.0;//,levels[i][2],
        extractHistogramValues(all_histogram_values, image,{0,0,0}); // For original images
        int j_k = 0;
        for (double stddev : stddev_values) {
            cv::Mat noisy_image = add_noise(image, stddev);
            extractHistogramValues(all_histogram_values, noisy_image,{0,0,0}); // For noisy images
            j_k += 1;
        }
        i_k += 1;
    }

    // Optionally print histogram values for each image
    for (size_t i = 0; i < all_histogram_values.size(); ++i) {
        std::cout << "Histogram for image " << i << ":" << std::endl;
        for (size_t j = 0; j < all_histogram_values[i].first.size(); ++j) {
            std::cout << all_histogram_values[i].first[j] << (j < all_histogram_values[i].first.size() - 1 ? ", " : "\n");
        }
        std::cout << std::endl;
    }

    // Теперь для каждого массива гистограммных значений...
    for (size_t i = 0; i < all_histogram_values.size(); ++i) {
        const auto& histogram_values = all_histogram_values[i].first;
        size_t part_size = histogram_values.size() / 3;

        // Разбиваем на три части и вычисляем среднее и дисперсию для каждой части
        for (int part = 0; part < 3; ++part) {
            // Вычисляем границы для части
            auto start_iter = histogram_values.begin() + part * part_size;
            auto end_iter = (part < 2) ? start_iter + part_size : histogram_values.end();
            
            // Считаем среднее для части
            std::vector<float> part_values(start_iter, end_iter);
            double mean_index = mean_index_of_vector(part_values)+part * part_size;
            // Считаем дисперсию для части
            double variance = variance_of_vector(part_values, mean_of_vector(part_values));

            // Выводим среднее и дисперсию для каждой части
            std::cout << "Image " << i << " - Part " << part << " - Mean: " << mean_index << ", Variance: " << variance << std::endl;
        }
    }

    // Show and save the final image
    cv::imshow("Test Image with Histograms and Noise", final_image);
    cv::waitKey(0);
    cv::imwrite("final_image.png", final_image);

    return 0;
}