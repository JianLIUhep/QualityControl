#ifndef QC_MODULE_ITSQCTRHESHOLDTASK_ITSQCTRHESHOLDTASKCHECK_H
#define QC_MODULE_ITSQCTRHESHOLDTASK_ITSQCTRHESHOLDTASKCHECK_H

#include "QualityControl/CheckInterface.h"
#include "QualityControl/MonitorObject.h"
#include "QualityControl/Quality.h"
#include <TPaveText.h>

/*namespace o2::quality_control_modules::itsrawtask
{
*/
//TPaveText* warning = new TPaveText(0.3, 0.2, 0.7, 0.45, "NDC");
namespace o2 {
namespace quality_control_modules {
namespace itsqctrhesholdtask {
//TPaveText* warning = new TPaveText(0.3, 0.2, 0.7, 0.45, "NDC");
class ITSQCTrhesholdTaskCheck : public o2::quality_control::checker::CheckInterface
{
	public:
		ITSQCTrhesholdTaskCheck() = default;
		~ITSQCTrhesholdTaskCheck() override = default;

		void configure(std::string name) override;
		Quality check(std::map<std::string, std::shared_ptr<MonitorObject>>* moMap) override;
		void beautify(std::shared_ptr<MonitorObject> mo, Quality checkResult = Quality::Null) override;
		std::string getAcceptedType() override;
		TPaveText* warning = new TPaveText(0.3, 0.2, 0.7, 0.45, "NDC");
	private:
		

	ClassDefOverride(ITSQCTrhesholdTaskCheck, 1);
};

}
}
}

#endif
