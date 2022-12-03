#include "sheet.h"

#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::Sheet() {}
Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }
    CellBuilder cb(this, pos);
    UniqCellPtr cell = cb.CreateCell(text);

    if (sheet_.count(pos)) {
        dynamic_cast<Cell*>(sheet_.at(pos).get())->InvalidateCache();
    }

    sheet_.insert_or_assign(pos, std::move(cell));
    IncreasePrintArea(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }
    if (sheet_.count(pos)) {
        return sheet_.at(pos).get();
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Invalid position"s);
    }

    if (!sheet_.count(pos)) {
        return;
    }
    sheet_.erase(pos);

    if (pos.col < (min_print_area_.cols - 1) && (pos.row < min_print_area_.rows - 1)) { //Don't change print area
        return;
    }
    else if (pos.row == (min_print_area_.rows - 1) && pos.col < (min_print_area_.cols - 1)) {
        DecreasePrintAreaRow(pos);
    }
    else if (pos.row < (min_print_area_.rows - 1) && pos.col == (min_print_area_.cols - 1)) {
        DecreasePrintAreaCol(pos);  
    }
    else {
        DecreasePrintAreaRow(pos);
        DecreasePrintAreaCol(pos);
    }

    if (sheet_.empty()) {
        min_print_area_ = { 0, 0 };
    }
}

Size Sheet::GetPrintableSize() const {
    return min_print_area_;
}

void Sheet::PrintValue(std::ostream& output, Position pos) const {
    if (!sheet_.count(pos)) {
        output << ""s;
        return;
    }

    auto value = sheet_.at(pos).get()->GetValue();
    if (IsType<std::string>(value)) {
        output << std::get<std::string>(value);
    }
    else if (IsType<double>(value)) {
        output << std::get<double>(value);
    }
    else if (IsType<FormulaError>(value)) {
        output << std::get<FormulaError>(value);
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < min_print_area_.rows; ++i) {
        bool is_start = true;
        for (int j = 0; j < min_print_area_.cols; ++j) {
            Position pos{ i, j };
            if (is_start) {
                PrintValue(output, pos);
                is_start = false;
            }
            else {
                output << '\t';
                PrintValue(output, pos);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < min_print_area_.rows; ++i) {
        bool is_start = true;
        for (int j = 0; j < min_print_area_.cols; ++j) {
            Position pos{ i, j };
            if (is_start) {
                output << (sheet_.count(pos) ? (sheet_.at(pos).get()->GetText()) : ""s);
                is_start = false;
            }
            else {
                output << '\t';
                output << (sheet_.count(pos) ? (sheet_.at(pos).get()->GetText()) : ""s);
            }
        }
        output << '\n';
    }
}

void Sheet::IncreasePrintArea(Position pos) {
    if (pos.row >= min_print_area_.rows && pos.col >= min_print_area_.cols) {
        min_print_area_ = { ++pos.row, ++pos.col };
    }
    else if (pos.row < min_print_area_.rows && pos.col >= min_print_area_.cols) {
        min_print_area_.cols = ++pos.col;
    }
    else if (pos.row >= min_print_area_.rows && pos.col < min_print_area_.cols) {
        min_print_area_.rows = ++pos.row;
    }
}

void Sheet::DecreasePrintAreaRow(Position pos) {
    if (sheet_.empty()) {
        return;
    }
    for (int i = 0; i < min_print_area_.cols; ++i) {
        if (sheet_.count({ pos.row, i })) {
            return;
        }
    }
    --min_print_area_.rows;
    DecreasePrintAreaRow(Position{ --pos.row, pos.col });
}

void Sheet::DecreasePrintAreaCol(Position pos) {
    if (sheet_.empty()) {
        return;
    }
    for (int i = 0; i < min_print_area_.rows; ++i) {
        if (sheet_.count({ i, pos.col })) {
            return;
        }
    }
    --min_print_area_.cols;
    DecreasePrintAreaCol(Position{ pos.row, --pos.col });
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
