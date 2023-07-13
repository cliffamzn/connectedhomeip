/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <AppMain.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <instances/ActivatedCarbonFilterMonitoring.h>
#include <instances/HepaFilterMonitoring.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;

constexpr std::bitset<4> gHepaFilterFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                static_cast<uint32_t>(Feature::kWarning) };
constexpr std::bitset<4> gActivatedCarbonFeatureMap{ static_cast<uint32_t>(Feature::kCondition) |
                                                     static_cast<uint32_t>(Feature::kWarning) };

static HepaFilterMonitoringInstance HepafilterInstance(0x1, static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                                                DegradationDirectionEnum::kDown, true);
static ActivatedCarbonFilterMonitoringInstance ActivatedCarbonFilterInstance(0x1,
                                                                      static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()),
                                                                      DegradationDirectionEnum::kDown, true);

void ApplicationInit()
{
    HepafilterInstance.Init();
    ActivatedCarbonFilterInstance.Init();
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}
