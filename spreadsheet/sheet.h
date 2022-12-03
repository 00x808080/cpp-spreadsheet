#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>

using UniqCellPtr = std::unique_ptr<CellInterface>;

struct SheetHasher {
    uint64_t operator()(Position pos) const {
        return pos.row + pos.col;
    }
};

class Sheet : public SheetInterface {
public:

    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;
    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    void PrintValue(std::ostream& output, Position pos) const;

private:
    void IncreasePrintArea(Position pos);
    void DecreasePrintAreaRow(Position pos);
    void DecreasePrintAreaCol(Position pos);

    template <typename T>
    [[nodiscard]] bool IsType(const CellInterface::Value& value) const {
        return std::holds_alternative<T>(value);
    }
    
    std::unordered_map<Position, UniqCellPtr, SheetHasher> sheet_;
    Size min_print_area_ = { 0, 0 };
};
