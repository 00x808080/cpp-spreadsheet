#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {
            FillUniqCells();
        }
        catch (const FormulaException& fe) {
            throw fe;
        }
        catch (std::exception& e) {
            throw FormulaException(e.what());
        }
        catch (...) {
            throw FormulaException("UNKNOWN ERROR");
        }
        
        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            }   
            catch (const FormulaError& fe) {
                return fe;
            }
        }
        
        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            return cells_;
        }
    private:
        FormulaAST ast_;
        std::vector<Position> cells_;

        void FillUniqCells() {
            for (auto pos : ast_.GetCells()) {
                if (cells_.empty() || !(cells_.back() == pos)) {
                    cells_.emplace_back(pos);
                }
            }
        }
    };
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
