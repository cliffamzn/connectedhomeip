/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once

#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ResourceMonitoring {

/// This is an application level instance to handle HepaFilterMonitoringInstance commands according to the specific business logic.
class HepaFilterMonitoringInstance : public ResourceMonitoring::Instance
{
private:
    CHIP_ERROR AppInit() override;
    chip::Protocols::InteractionModel::Status OnResetCondition() override;

public:
    HepaFilterMonitoringInstance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeature,
                                 ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection, bool aResetConditionCommandSupported) :
    Instance(aEndpointId, aClusterId, aFeature, aDegradationDirection, aResetConditionCommandSupported){};

    ~HepaFilterMonitoringInstance() override = default;
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip