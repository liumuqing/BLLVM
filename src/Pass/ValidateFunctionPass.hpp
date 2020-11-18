#include "Pass/Pass.hpp"

class ValidateFunctionPass:
	public PassInfoMixin<ValidateFunctionPass>,
	virtual public Analysis,
	virtual public FunctionPass{
public:
	bool run() override final;
	bool isValidated() const {
		return validated_;
	}
private:
	bool validated_ = false;
};
