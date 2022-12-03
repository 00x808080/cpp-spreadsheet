#pragma once

#include "common.h"
#include "formula.h"
#incude <optional>

class Impl {
public:
    virtual CellInterface::Value GetValue() const = 0;
    virtual std::string GetText() const = 0;
};

class EmptyImpl: public Impl {
public:
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
};

class TextImpl : public Impl {
public:
    TextImpl(std::string text);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;
private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string expression);
    CellInterface::Value GetValue() const override;
    std::string GetText() const override;

private:
    std::unique_ptr<FormulaInterface> formula_;

    template <typename T>
    [[nodiscard]] bool Is(const FormulaInterface::Value& value) const {
        return std::holds_alternative<T>(value);
    }
};

class Cell : public CellInterface {
public:
    Cell(std::unique_ptr<Impl> impl_);
    ~Cell();

    Value GetValue() const override;
    std::string GetText() const override;
	std::vector<Position> GetReferencedCells();
	void InvalidateCache();
	
private:
    std::unique_ptr<Impl> impl_ = nullptr;
	std::vector<Position> list_depended_;
	std::optional<CellInterface::Value> cache_;
};