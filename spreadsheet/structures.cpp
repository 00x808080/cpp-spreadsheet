#include "common.h"

#include <cctype>
#include <sstream>
#include <cmath>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 8;
const int MAX_POS_LETTER_COUNT = 3;

const Position Position::NONE = { -1, -1 };

bool Position::operator==(const Position rhs) const {
	return col == rhs.col && row == rhs.row;
}

bool Position::operator<(const Position rhs) const {
	return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
	return (-1 < col && col < MAX_COLS) && (-1 < row && row < MAX_ROWS);
}

bool Size::operator==(Size rhs) const {
	return cols == rhs.cols && rows == rhs.rows;
}

int ConvertToCol(std::string_view str) {
	int degree = str.size() - 1;

	int col = 0;

	for (char c : str) {
		col += (static_cast<int>(c) - 64) * std::pow(LETTERS, degree);
		--degree;
	}

	return col;
}

void ConvertColToString(int col, std::string& upper_char) {
	if (col < LETTERS) {
		char c = static_cast<char>(col + 65);
		upper_char.insert(upper_char.begin(), c);
		return;
	}
	int val_div = col / LETTERS;
	int remainder = col - (val_div * LETTERS);

	upper_char.insert(upper_char.begin(), static_cast<char>(remainder + 65));

	return ConvertColToString(--val_div, upper_char);
}

std::string Position::ToString() const {
	if (!IsValid()) {
		return {};
	}
	std::string upper_char;
	ConvertColToString(col, upper_char);

	std::string numbers(std::to_string(row + 1));

	return upper_char + numbers;
}

Position Position::FromString(std::string_view str) {
	if (str.empty() || std::isdigit(str.front()) || std::islower(str.front()) ||
		(str.size() > MAX_POSITION_LENGTH)) {
		return Position::NONE;
	}

	std::string upper_char;
	auto it = str.begin();

	while (it != str.end() ? std::isupper(*it) : false) {
		upper_char.push_back(*it);
		++it;
	}

	if (upper_char.size() > MAX_POS_LETTER_COUNT) {
		return Position::NONE;
	}

	std::string numbers;
	while (it != str.end() ? std::isdigit(*it) : false) {
		numbers.push_back(*it);
		++it;
	}

	if (numbers.empty() || it != str.end() ||
		(numbers.size() > (MAX_POSITION_LENGTH - MAX_POS_LETTER_COUNT))) {
		return Position::NONE;
	}

	int row = std::stoi(numbers);
	int col = ConvertToCol(upper_char);

	return (col > MAX_COLS || row > MAX_ROWS) ? Position::NONE : Position{ --row, --col };
}