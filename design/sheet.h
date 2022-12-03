#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <list>
#include <unordered_map>

class Sheet : public SheetInterface {
public:
    using UniqCellPtr = std::unique_ptr<CellInterface>;
    using ColIterator = std::list<std::list<UniqCellPtr>>::iterator;
    using RowIterator = std::list<UniqCellPtr>::iterator;

    Sheet();
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

    
private:
    std::unordered_map<std::string, ColIterator> access_to_col_;
    std::unordered_map<std::string, RowIterator> access_to_row_;
    std::list<std::list<UniqCellPtr>> sheet_;
    Size min_print_area_ = { 0, 0 };
	
    void AddCol(Position pos);
    void AddRow(Position pos);

    void ChangePrintAreaByCol();
    void ChangePrintAreaByRow();
	
    std::unique_ptr<Impl> impl_ GetTypeCell(std::string text);
    void CheckCyclicDependencies();
};

template <typename T>
[[nodiscard]] bool Is(const CellInterface::Value& value) {
    return std::holds_alternative<T>(value);
}

