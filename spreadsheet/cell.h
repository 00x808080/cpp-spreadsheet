#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>

class Sheet;
class Cell;
using UniqCellPtr = std::unique_ptr<CellInterface>;

struct Hasher {
    uint64_t operator()(Position pos) const {
        return pos.row + pos.col;
    }
};

class CellBuilder {
public:
    CellBuilder(Sheet* sheet, Position pos);
    UniqCellPtr CreateCell(std::string text);

private:
    void CheckCyclicDependencies(const std::vector<Position>& vec_pos, Position vertex);
    
    Sheet* sheet_ = nullptr;
    Position current_pos_;
    std::unordered_map<Position, bool, Hasher> bypass_list_;
};

enum class TypeCell {
    EmptyImpl,
    TextImpl,
    FormulaImpl
};

class Impl {
public:
    virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::string GetText() const = 0;
    virtual TypeCell GetTypeCell() const = 0;
    virtual std::vector<Position> GetCells() const = 0;
};

class EmptyImpl: public Impl {
public:
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeCell GetTypeCell() const;
    std::vector<Position> GetCells() const override;
};

class TextImpl : public Impl {
public:
    TextImpl(std::string text);
    
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeCell GetTypeCell() const;
    std::vector<Position> GetCells() const override;

private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string expression);
    
    CellInterface::Value GetValue(const SheetInterface& sheet) const override;
    std::string GetText() const override;
    TypeCell GetTypeCell() const;
    std::vector<Position> GetCells() const override;

private:
    template <typename T>
    [[nodiscard]] bool IsValue(const FormulaInterface::Value& value) const {
        return std::holds_alternative<T>(value);
    }
    
    std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    friend class CellBuilder;
    
    ~Cell();

    void SetDepended(Position pos);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;
    TypeCell GetTypeCell() const;

    Position GetPosition() const;
    void InvalidateCache();
    void SetCache(CellInterface::Value value);

private:
    Cell(const Sheet& sheet, std::unique_ptr<Impl> impl, Position pos);

    const Sheet& sheet_;    
    std::unique_ptr<Impl> impl_ = nullptr;
    Position own_position_;
    std::unordered_set<Position, Hasher> list_depended_;
    std::optional<CellInterface::Value> cache_;
};
