#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

using namespace std::literals;

CellBuilder::CellBuilder(Sheet* sheet, Position pos)
    : sheet_(sheet)
    , current_pos_(pos) {
}

UniqCellPtr CellBuilder::CreateCell(std::string text) {
	std::unique_ptr<Impl> impl = nullptr;
	if (text.empty()) {
		impl = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text.front() == FORMULA_SIGN) {
		if (text[1] == ESCAPE_SIGN) {
			impl = std::make_unique<TextImpl>(text.substr(1, text.size()));
		}
		else {
			impl = std::make_unique<FormulaImpl>(text.substr(1, text.size()));
			const std::vector<Position>& vec_pos = dynamic_cast<FormulaImpl*>(impl.get())->GetCells();
			CheckCyclicDependencies(vec_pos, current_pos_);	//throw exceptions CircularDependencyException
		}
	}
	else {
		impl = std::make_unique<TextImpl>(text);
	}
	Cell* cell = new Cell(*sheet_, std::move(impl), std::move(current_pos_));
	return std::unique_ptr<CellInterface>(cell);
}

void CellBuilder::CheckCyclicDependencies(const std::vector<Position>& vec_pos, Position vertex) {
	if (bypass_list_[vertex] != 0) {
		return;
	}
	bypass_list_[vertex] = 1;

	for (const auto pos : vec_pos) {
		if (!(pos == current_pos_)) {
			CellInterface* cell_inter = sheet_->GetCell(pos);
				if (cell_inter == nullptr) {
					sheet_->SetCell(pos, ""s);
					continue;
				}
			dynamic_cast<Cell*>(cell_inter)->SetDepended(vertex);
			const std::vector<Position> next_vec_pos = cell_inter->GetReferencedCells();
			CheckCyclicDependencies(next_vec_pos, pos);
		}
		else {
			throw CircularDependencyException("ERROR Circular Dependency"s);
		}
	}
}

//EmptyImpl
CellInterface::Value EmptyImpl::GetValue([[maybe_unused]] const SheetInterface& sheet) const {
	return CellInterface::Value();
}

std::string EmptyImpl::GetText() const {
	return std::string();
}

TypeCell EmptyImpl::GetTypeCell() const {
	return TypeCell::EmptyImpl;
}

std::vector<Position> EmptyImpl::GetCells() const {
	return std::vector<Position>();
}

//TextImpl
TextImpl::TextImpl(std::string text): text_(std::move(text)) {}

CellInterface::Value TextImpl::GetValue([[maybe_unused]] const SheetInterface& sheet) const {
	if (text_.empty()) {
		return {};
	}
	return text_.front() == ESCAPE_SIGN ? text_.substr(1, text_.size()) : text_;
}

std::string TextImpl::GetText() const {
	return text_;
}

TypeCell TextImpl::GetTypeCell() const {
	return TypeCell::TextImpl;
}

std::vector<Position> TextImpl::GetCells() const {
	return std::vector<Position>();
}

//FormulaImpl
FormulaImpl::FormulaImpl(std::string expression): formula_(std::move(ParseFormula(expression))) {}

CellInterface::Value FormulaImpl::GetValue(const SheetInterface& sheet) const {
	FormulaInterface::Value value = formula_->Evaluate(sheet);
	if (IsValue<FormulaError>(value)) {
		return std::get<FormulaError>(value);
	}
	return std::get<double>(value);
}

std::string FormulaImpl::GetText() const {
	return '=' + formula_->GetExpression();
}

TypeCell FormulaImpl::GetTypeCell() const {
	return TypeCell::FormulaImpl;
}

std::vector<Position> FormulaImpl::GetCells() const {
	return formula_->GetReferencedCells();
}

//Cell
Cell::Cell(const Sheet& sheet, std::unique_ptr<Impl> impl, Position pos)
    : sheet_(sheet)
    , impl_(std::move(impl))
    , own_position_(pos) {
}

Cell::~Cell() {}

void Cell::Clear() {}

Cell::Value Cell::GetValue() const {
	if (cache_.has_value()) {
		return cache_.value();
	}
	CellInterface::Value value = impl_->GetValue(sheet_);
	const_cast<Cell*>(this)->cache_ = value;
	return value;
}
std::string Cell::GetText() const {
	return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetCells();
}

void Cell::SetDepended(Position pos) {
	if (!list_depended_.count(pos)) {
		list_depended_.insert(pos);
	}
}

Position Cell::GetPosition() const {
	return own_position_;
}

void Cell::InvalidateCache() {
	if (!cache_.has_value()) {
		return;
	}
	cache_.reset();

	for (auto pos : list_depended_) {
		CellInterface* cell_inter = const_cast<CellInterface*>(sheet_.GetCell(pos));
		Cell* current_cell = dynamic_cast<Cell*>(cell_inter);
		current_cell->InvalidateCache();
	}
}

TypeCell Cell::GetTypeCell() const {
	return impl_->GetTypeCell();
}

void Cell::SetCache(CellInterface::Value value) {
	cache_ = value;
}
