/*
 * Copyright (c) 2017-2019 THL A29 Limited, a Tencent company. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TENCENTCLOUD_CORE_ABSTRACTCLIENT_H_
#define TENCENTCLOUD_CORE_ABSTRACTCLIENT_H_

#include <tencentcloud/core/profile/ClientProfile.h>
#include <tencentcloud/core/http/HttpClient.h>
#include "Credential.h"
#include "AbstractModel.h"
#include "AsyncCallerContext.h"
#include <map>
#include <future>

namespace TencentCloud
{
    class AbstractClient
    {
    public:
        typedef std::function<void(const HttpClient::HttpResponseOutcome&)> AsyncCallback;

        AbstractClient(const std::string &endpoint, const std::string &version, const Credential &credential, const std::string &region);
        AbstractClient(const std::string &endpoint, const std::string &version, const Credential &credential, const std::string &region, const ClientProfile &profile);
        ~AbstractClient();

        void SetRegion(const std::string &region);
        std::string GetRegion() const;
        void SetClientProfile(const ClientProfile &profile);
        ClientProfile GetClientProfile() const;
        void SetCredential(const Credential &credential);
        Credential GetCredential() const;
        void SetNetworkProxy(const NetworkProxy &proxy);
        void SetHeader(const std::map<std::string, std::string> &headers);
        HttpClient::HttpResponseOutcome MakeRequestJson(const std::string &actionName, const std::string &params);
        HttpClient::HttpResponseOutcome MakeRequestOctetStream(const std::string &actionName, std::map<std::string, std::string> &headers, const std::string &body);

    protected:

        HttpClient::HttpResponseOutcome MakeRequest(const AbstractModel& request, const std::string &actionName);

        template<typename ClientType, typename ReqType, typename HandlerType, typename RespType, typename OutcomeType>
        void MakeRequestAsync(ClientType* client, const ReqType& request, const std::string &actionName, HandlerType handler, const std::shared_ptr<const AsyncCallerContext>& context)
        {
            auto callback = [this, client, request, handler, context](const HttpClient::HttpResponseOutcome& outcome)
            {
                auto response = HandleResponse<RespType, OutcomeType>(outcome);
                handler(client, request, response, context);
            };
            DoRequestAsync(request, actionName, callback);
        }

        HttpClient::HttpResponseOutcome DoRequest(const std::string &actionName, const std::string &body, std::map<std::string, std::string> &headers);
        void DoRequestAsync(AbstractModel request, const std::string &actionName, const AsyncCallback& handler);

        void GenerateSignature(HttpRequest &request);

        template <typename RespType, typename OutcomeType>
        OutcomeType HandleResponse(const HttpClient::HttpResponseOutcome &outcome)
        {
            OutcomeType response;
            if (outcome.IsSuccess()) 
            {
                auto r = outcome.GetResult();
                std::string payload = std::string(r.Body(), r.BodySize());
                RespType rsp = RespType();
                auto o = rsp.Deserialize(payload);
                if (o.IsSuccess())
                    response = OutcomeType(rsp);
                else
                    response = OutcomeType(o.GetError());
            } 
            else
            {
                response = OutcomeType(outcome.GetError());
            }
            return response;
        };

    private:
        Credential m_credential;
        ClientProfile m_clientProfile;
        std::string m_endpoint;
        std::string m_region;
        std::string m_sdkVersion;
        std::string m_apiVersion;
        HttpClient *m_httpClient;
        std::string m_service;
        std::map<std::string, std::string> m_headers;
    };
}

#endif // !TENCENTCLOUD_CORE_ABSTRACTCLIENT_H_
