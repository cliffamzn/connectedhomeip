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
     * This method may be overwritten by the SDK User, if the default behaviour is not desired.
     * Preferably, the SDK User should implement the PreResetCondition() and PostResetCondition() methods instead.
     *
     * The cluster implementation will handle all of the resets needed by the spec.
     * - Update the Condition attribute according to the DegradationDirection (if supported)
     * - Update the ChangeIndicator attribute to kOk
     * - Update the LastChangedTime attribute (if supported)
     *
     * The return value will depend on the PreResetCondition() and PostResetCondition() method, if one of them does not return
     * Success, this method will return the failure as well.
     * @return Status::Success      If the command was handled successfull
     * @return All Other            PreResetCondition() or PostResetCondition() failed, these are application specific.
     */
    virtual chip::Protocols::InteractionModel::Status OnResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something before the reset.
     * If there are some internal states of the devices or some specific methods that must be called, that are needed for the reset
     * and that can fail, they should be done here and not in PostResetCondition().
     *
     * @return Status::Success      All good, the reset may proceed.
     * @return All Other            The reset should not proceed. The reset command will fail.
     */
    virtual chip::Protocols::InteractionModel::Status PreResetCondition();

    /**
     * This method may be overwritten by the SDK User, if the SDK User wants to do something after the reset.
     * If this fails, the attributes will already be updated, so the SDK User should not do something here
     * that can fail and that will affect the state of the device. Do the checks in the PreResetCondition() method instead.
     *
     * @return Status::Success      All good
     * @return All Other            Something went wrong. The attributes will already be updated. But the reset command will report
     *                              the failure.
     */
    virtual chip::Protocols::InteractionModel::Status PostResetCondition();

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
