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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/AttributePersistenceProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/resource-monitoring-server/resource-monitoring-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <platform/DiagnosticDataProvider.h>

// using namespace std;
using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ResourceMonitoring;
using chip::Protocols::InteractionModel::Status;

using BootReasonType = GeneralDiagnostics::BootReasonEnum;

namespace chip {
namespace app {
namespace Clusters {
namespace ResourceMonitoring {

void Instance::LoadPersistentAttributes()
{
    CHIP_ERROR err = chip::app::GetAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LastChangedTime::Id), mLastChangedTime);
    if (err == CHIP_NO_ERROR)
    {
        if (mLastChangedTime.IsNull())
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as null");
        }
        else
        {
            ChipLogDetail(Zcl, "ResourceMonitoring: Loaded LastChangedTime as %lu",
                          (long unsigned int) mLastChangedTime.Value()); // on some platforms uint32_t is a long, cast it to
                                                                         // unsigned long on all platforms to prevent CI errors
        }
    }
    else
    {
        // If we cannot find the previous LastChangedTime, we will assume it to be null.
        ChipLogDetail(Zcl, "ResourceMonitoring: Unable to load the LastChangedTime from the KVS. Assuming null");
    }
}

CHIP_ERROR Instance::Init()
{
    ChipLogDetail(Zcl, "ResourceMonitoring: Init");
    // Check that the cluster ID given is a valid mode select alias cluster ID.
    VerifyOrDie(IsValidAliasCluster());

    // Check if the cluster has been selected in zap
    VerifyOrDie(emberAfContainsServer(mEndpointId, mClusterId));

    LoadPersistentAttributes();

    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    ChipLogDetail(Zcl, "ResourceMonitoring: calling AppInit()");
    ReturnErrorOnFailure(AppInit());

    return CHIP_NO_ERROR;
}

// This method is called by the interaction model engine when a command destined for this instance is received.
void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "ResourceMonitoring Instance::InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case ResourceMonitoring::Commands::ResetCondition::Id:
        ChipLogDetail(Zcl, "ResourceMonitoring::Commands::ResetCondition");

        HandleCommand<ResourceMonitoring::Commands::ResetCondition::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResetCondition(ctx, commandData); });
        break;
    }
}

// List the commands supported by this instance.
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster,
                                               CommandHandlerInterface::CommandIdCallback callback, void * context)
{
    ChipLogDetail(Zcl, "resourcemonitoring: EnumerateAcceptedCommands");
    if (mResetConditionCommandSupported)
    {
        callback(ResourceMonitoring::Commands::ResetCondition::Id, context);
    }

    return CHIP_NO_ERROR;
}

bool Instance::HasFeature(ResourceMonitoring::Feature aFeature) const
{
    return ((mFeatureMap & to_underlying(aFeature)) != 0);
}

// Implements the read functionality for non-standard attributes.
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Condition::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mCondition));
        break;
    }
    case Attributes::FeatureMap::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mFeatureMap));
        break;
    }
    case Attributes::DegradationDirection::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mDegradationDirection));
        break;
    }
    case Attributes::ChangeIndication::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mChangeIndication));
        break;
    }
    case Attributes::InPlaceIndicator::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mInPlaceIndicator));
        break;
    }
    case Attributes::LastChangedTime::Id: {
        ReturnErrorOnFailure(aEncoder.Encode(mLastChangedTime));
        break;
    }
    }
    return CHIP_NO_ERROR;
}

// Implements checking before attribute writes.
CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::LastChangedTime::Id: {
        DataModel::Nullable<uint32_t> newLastChangedTime;
        ReturnErrorOnFailure(aDecoder.Decode(newLastChangedTime));
        UpdateLastChangedTime(newLastChangedTime);
        break;
    }
    }
    return CHIP_NO_ERROR;
}

chip::Protocols::InteractionModel::Status Instance::UpdateCondition(uint8_t aNewCondition)
{
    auto oldCondition = mCondition;
    mCondition        = aNewCondition;
    if (mCondition != oldCondition)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::Condition::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status
Instance::UpdateChangeIndication(chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum aNewChangeIndication)
{
    if (aNewChangeIndication == chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum::kWarning)
    {
        if (!HasFeature(ResourceMonitoring::Feature::kWarning))
        {
            return Protocols::InteractionModel::Status::InvalidValue;
        }
    }
    auto oldChangeIndication = mChangeIndication;
    mChangeIndication        = aNewChangeIndication;
    if (mChangeIndication != oldChangeIndication)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::ChangeIndication::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Instance::UpdateInPlaceIndicator(bool aNewInPlaceIndicator)
{
    auto oldInPlaceIndicator = mInPlaceIndicator;
    mInPlaceIndicator        = aNewInPlaceIndicator;
    if (mInPlaceIndicator != oldInPlaceIndicator)
    {
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::InPlaceIndicator::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

chip::Protocols::InteractionModel::Status Instance::UpdateLastChangedTime(DataModel::Nullable<uint32_t> aNewLastChangedTime)
{
    auto oldLastchangedTime = mLastChangedTime;
    mLastChangedTime        = aNewLastChangedTime;
    if (mLastChangedTime != oldLastchangedTime)
    {
        chip::app::GetAttributePersistenceProvider()->WriteScalarValue(
            ConcreteAttributePath(mEndpointId, mClusterId, Attributes::LastChangedTime::Id), mLastChangedTime);
        MatterReportingAttributeChangeCallback(mEndpointId, mClusterId, Attributes::LastChangedTime::Id);
    }
    return Protocols::InteractionModel::Status::Success;
}

uint8_t Instance::GetCondition() const
{
    return mCondition;
}
chip::app::Clusters::ResourceMonitoring::ChangeIndicationEnum Instance::GetChangeIndication() const
{
    return mChangeIndication;
}

chip::app::Clusters::ResourceMonitoring::DegradationDirectionEnum Instance::GetDegradationDirection() const
{
    return mDegradationDirection;
}

bool Instance::GetInPlaceIndicator() const
{
    return mInPlaceIndicator;
}

DataModel::Nullable<uint32_t> Instance::GetLastChangedTime() const
{
    return mLastChangedTime;
}

bool Instance::IsValidAliasCluster() const
{
    for (unsigned int AliasedCluster : AliasedClusters)
    {
        if (mClusterId == AliasedCluster)
        {
            return true;
        }
    }
    return false;
}

void Instance::HandleResetCondition(HandlerContext & ctx,
                                    const ResourceMonitoring::Commands::ResetCondition::DecodableType & commandData)
{

    Status resetConditionStatus = OnResetCondition();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, resetConditionStatus);
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    ChipLogDetail(Zcl, "ResourceMonitoring: HandleCommand");
    if (handlerContext.mCommandHandled || (handlerContext.mRequestPath.mCommandId != RequestT::GetCommandId()))
    {
        return;
    }

    RequestT requestPayload;

    // If the command matches what the caller is looking for, let's mark this as being handled
    // even if errors happen after this. This ensures that we don't execute any fall-back strategies
    // to handle this command since at this point, the caller is taking responsibility for handling
    // the command in its entirety, warts and all.
    handlerContext.SetCommandHandled();

    if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
    {
        handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
        return;
    }

    func(handlerContext, requestPayload);
}

} // namespace ResourceMonitoring
} // namespace Clusters
} // namespace app
} // namespace chip
