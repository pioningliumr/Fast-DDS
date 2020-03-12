// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file Topic.cpp
 */

#include <fastdds/topic/TopicImpl.hpp>
#include <fastdds/dds/topic/TopicListener.hpp>
#include <fastdds/dds/topic/qos/DataWriterQos.hpp>

namespace eprosima {
using ReturnCode_t = fastrtps::types::ReturnCode_t;
namespace fastdds {
namespace dds {

TopicImpl::TopicImpl(
        DomainParticipant* dp,
        const TopicQos& qos,
        TopicListener* listener)
    : user_topic_(nullptr)
    , listener_(listener)
    , qos_(qos)
    , participant_(dp)
{
}

void TopicImpl::disable()
{
    set_listener(nullptr);
}

TopicImpl::~TopicImpl()
{
    delete user_topic_;
}

fastrtps::TopicAttributes TopicImpl::get_topic_attributes() const
{
    return qos_.topic_attr;
}

ReturnCode_t TopicImpl::get_qos(
        TopicQos& qos) const
{
    qos = qos_;
    return ReturnCode_t::RETCODE_OK;
}

const TopicQos& TopicImpl::get_qos() const
{
    return qos_;
}

ReturnCode_t TopicImpl::set_qos(
        const TopicQos& qos)
{
    if (qos.checkQos())
    {
        qos_ = qos;
        return ReturnCode_t::RETCODE_OK;
    }
    return ReturnCode_t::RETCODE_INCONSISTENT_POLICY;
}

TopicListener* TopicImpl::get_listener() const
{
    return listener_;
}

ReturnCode_t TopicImpl::set_listener(
        TopicListener* a_listener)
{
    listener_ = a_listener;
    return ReturnCode_t::RETCODE_OK;
}

DomainParticipant* TopicImpl::get_participant() const
{
    return participant_;
}

std::vector<DataWriter*>* TopicImpl::get_writers()
{
    return &writers_;
}

std::vector<DataReader*>* TopicImpl::get_readers()
{
    return &readers_;
}

ReturnCode_t TopicImpl::get_inconsistent_topic_status(
        InconsistentTopicStatus& status)
{
    status = status_;
    status_.total_count_change = 0;
    user_topic_->get_statuscondition()->set_status_as_read(::dds::core::status::StatusMask::inconsistent_topic());
    return ReturnCode_t::RETCODE_OK;
}

void TopicImpl::new_inconsistent_topic(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    status_.total_count++;
    status_.total_count_change++;
    entity_with_inconsistent_topic_.push_back(handle);

    if (listener_ != nullptr)
    {
        listener_->on_inconsistent_topic(user_topic_, status_);
        if (!user_topic_->get_statuscondition()->is_attached())
        {
            status_.total_count_change = 0;
        }
    }
}

bool TopicImpl::is_entity_already_checked(
        const fastrtps::rtps::InstanceHandle_t& handle)
{
    auto it = std::find(entity_with_inconsistent_topic_.begin(), entity_with_inconsistent_topic_.end(), handle);
    if (it != entity_with_inconsistent_topic_.end())
    {
        return true;
    }
    return false;
}

} // namespace dds
} // namespace fastdds
} // namespace eprosima
