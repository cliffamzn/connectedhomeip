#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/config.h>
#include <instances/InkCartridgeMonitoring.h>

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::InkCartridgeMonitoring;
using chip::Protocols::InteractionModel::Status;

using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

//-- Ink Cartridge Monitoring instance functions
CHIP_ERROR InkCartridgeMonitoringInstance::AppInit()
{
    ChipLogDetail(Zcl, "InkCartridgeMonitoringInstance::Init()");
    return CHIP_NO_ERROR;
}

Status InkCartridgeMonitoringInstance::OnResetCondition()
{
    ChipLogDetail(Zcl, "InkCartridgeMonitoringInstance::OnResetCondition()");

    if ( GetDegradationDirection() == DegradationDirectionEnum::kDown)
    {
        UpdateCondition(100);
    }
    else if ( GetDegradationDirection() == DegradationDirectionEnum::kUp)
    {
        UpdateCondition(0);
    }
    if (emberAfContainsAttribute(0x1, Clusters::InkCartridgeMonitoring::Id, Attributes::LastChangedTime::Id))
    {
        System::Clock::Milliseconds64 currentUnixTimeMS;
        System::Clock::ClockImpl clock;
        CHIP_ERROR err = clock.GetClock_RealTimeMS(currentUnixTimeMS);
        if (err == CHIP_NO_ERROR)
        {
            // If the system has given us a wall clock time, we must use it or
            // fail.  Conversion failures here are therefore always an error.
            System::Clock::Seconds32 currentUnixTime = std::chrono::duration_cast<System::Clock::Seconds32>(currentUnixTimeMS);
            UpdateLastChangedTime(DataModel::MakeNullable(currentUnixTime.count()));
        }
    }

    return Status::Success;
}