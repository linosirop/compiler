// src/main.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "lexer/lexer.h"

int main(int argc, char* argv[]) {
    if (argc != 6 ||
        std::string(argv[1]) != "lex" ||
        std::string(argv[2]) != "--input" ||
        std::string(argv[4]) != "--output") {

        std::cerr << "Usage: compiler lex --input <input_file.src> --output <output_file.txt>\n";
        std::cerr << "Example:\n";
        std::cerr << "  compiler lex --input examples/hello.src --output tokens.txt\n";
        return 1;
    }

    std::string input_path = argv[3];
    std::string output_path = argv[5];

    // Читаем весь исходный файл
    std::ifstream input_file(input_path, std::ios::binary);
    if (!input_file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << input_path << "\n";
        return 1;
    }

    std::string source(
        (std::istreambuf_iterator<char>(input_file)),
        std::istreambuf_iterator<char>()
    );

    // Запускаем лексер
    Lexer lexer(source);

    // Проверяем, были ли ошибки во время лексического анализа
    const auto& errors = lexer.get_errors();
    if (!errors.empty()) {
        std::cerr << "Обнаружены ошибки лексического анализа:\n";
        for (const auto& err : errors) {
            std::cerr << err << "\n";
        }
        return 1;
    }

    // Записываем токены в выходной файл
    std::ofstream output_file(output_path);
    if (!output_file.is_open()) {
        std::cerr << "Ошибка: не удалось создать/открыть файл " << output_path << "\n";
        return 1;
    }

    const auto& tokens = lexer.all_tokens();

    for (const auto& tok : tokens) {
        output_file << tok.to_string() << "\n";
    }

    output_file.close();

    std::cout << "Токены успешно записаны в " << output_path << "\n";
    std::cout << "Всего токенов: " << tokens.size() << "\n";

    return 0;
}