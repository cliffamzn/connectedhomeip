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

#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>
#include <stdint.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

class Instance : public CommandHandlerInterface, public AttributeAccessInterface
{

public:
    /**
     * Initialise the Resource Monitoring cluster.
     *
     * @die                                     If the cluster ID given is not a valid Resource Monitoring cluster ID.
     * @die                                     If the endpoint and cluster ID have not been enabled in zap.
     * @return CHIP_ERROR_INVALID_ARGUMENT      If the CommandHandler or Attribute Handler could not be registered.
     * @return CHIP_ERROR_INCORRECT_STATE       If the CommandHandler was already registered
     * @return CHIP_ERROR_INCORRECT_STATE       If the registerAttributeAccessOverride fails.
     * @return CHIP_ERROR                       If the AppInit() method returned an error. This is application specific.
     *
     * @return CHIP_NO_ERROR                    If the cluster was initialised successfully.
     */
    CHIP_ERROR Init();

    /**
     * Checks if the given feature is supported by the cluster.
     * @param feature   The aFeature to check.
     *
     * @return true     If the feature is supported.
     * @return false    If the feature is not supported.
     */
    bool HasFeature(ResourceMonitoring::Feature aFeature) const;

    // Attribute setters
    chip::Protocols::InteractionModel::Status UpdateCondition(uint8_t aNewCondition);
    chip::Protocols::InteractionModel::Status UpdateChangeIndication(ChangeIndicationEnum aNewChangeIndication);
    chip::Protocols::InteractionModel::Status UpdateInPlaceIndicator(bool aNewInPlaceIndicator);
    chip::Protocols::InteractionModel::Status UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime);

    // Attribute getters
    uint8_t GetCondition() const;
    ChangeIndicationEnum GetChangeIndication() const;
    DegradationDirectionEnum GetDegradationDirection() const;
    bool GetInPlaceIndicator() const;
    DataModel::Nullable<uint32_t> GetLastChangedTime() const;
    EndpointId GetEndpointId() const { return mEndpointId; }

    /**
     * Creates a resource monitoring cluster instance. The Init() method needs to be called for this instance to be registered and
     * called by the interaction model at the appropriate times.
     * @param aEndpointId                       The endpoint on which this cluster exists. This must match the zap configuration.
     * @param aClusterId                        The ID of the ResourceMonitoring aliased cluster to be instantiated.
     * @param aFeatureMap                       The feature map of the cluster.
     * @param aDegradationDirection             The degradation direction of the cluster.
     * @param aResetConditionCommandSupported   Whether the ResetCondition command is supported by the cluster.
     */
    Instance(EndpointId aEndpointId, ClusterId aClusterId, uint32_t aFeatureMap,
             ResourceMonitoring::Attributes::DegradationDirection::TypeInfo::Type aDegradationDirection,
             bool aResetConditionCommandSupported) :
        CommandHandlerInterface(Optional<EndpointId>(aEndpointId), aClusterId),
        AttributeAccessInterface(Optional<EndpointId>(aEndpointId), aClusterId), mEndpointId(aEndpointId), mClusterId(aClusterId),
        mDegradationDirection(aDegradationDirection), mFeatureMap(aFeatureMap),
        mResetConditionCommandSupported(aResetConditionCommandSupported)
    {}

    ~Instance() = default;

    // Not copyable or movable
    Instance(const Instance &) = delete;
    Instance & operator=(const Instance &) = delete;
    Instance(Instance &&)                  = delete;
    Instance & operator=(Instance &&) = delete;

    // The following methods should be overridden by the SDK user to implement the business logic of their application

    /**
     * This init method will be called during Resource Monitoring Server initialization after the instance information has been
     * validated and the instance has been registered. This method should be overridden by the SDK user to initialize the
     * application logic.
     *
     * @return CHIP_NO_ERROR    If the application was initialized successfully. All other values will cause the initialization to
     * fail.
     */
    virtual CHIP_ERROR AppInit() = 0;

    /**
     * This method is to be overridden by a user implemented method to handle the application specifics of the ResetCondition
     * command.
     * The cluster implementation will not handle any of the resets needed by the spec. The user of the SDK will need to implement
     * the logic to reset:
     * - The Condition attribute (if supported)
     * - The ChangeIndicator attribute
     * - The LastChangedTime attribute (if supported)
     *
     * Upon receipt, the device SHALL reset the Condition and ChangeIndicator attributes, indicating full resource availability and
     * readiness for use, as initially configured. Invocation of this command MAY cause the LastChangedTime to be updated
     * automatically based on the clock of the server, if the server supports setting the attribute.
     *
     * @return Status::Success      If the command was handled successfully, all other will cause the command to fail.
     */
    virtual chip::Protocols::InteractionModel::Status OnResetCondition() = 0;

private:
    EndpointId mEndpointId{};
    ClusterId mClusterId{};

    // attribute Data Store
    chip::Percent mCondition                       = 100;
    DegradationDirectionEnum mDegradationDirection = DegradationDirectionEnum::kDown;
    ChangeIndicationEnum mChangeIndication         = ChangeIndicationEnum::kOk;
    bool mInPlaceIndicator                         = true;
    DataModel::Nullable<uint32_t> mLastChangedTime;

    uint32_t mFeatureMap;

    bool mResetConditionCommandSupported = false;

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & ctx) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;

    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func);

    void LoadPersistentAttributes();

    /**
     * This checks if the clusters instance is a valid ResourceMonitoring cluster based on the AliasedClusters list.
     * @return true     if the cluster is a valid ResourceMonitoring cluster.
     */
    bool IsValidAliasCluster() const;

    /**
     * Internal method to handle the ResetCondition command.
     */
    void HandleResetCondition(HandlerContext & ctx,
                              const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData);
};

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
