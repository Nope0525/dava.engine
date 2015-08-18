/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_ASSET_CACHE_CLIENT_H__
#define __DAVAENGINE_ASSET_CACHE_CLIENT_H__

#include "Base/BaseTypes.h"

#include "AssetCache/TCPConnection/TCPConnection.h"
#include "AssetCache/CacheItemKey.h"
#include "Network/Base/AddressResolver.h"

namespace DAVA {

class TCPClient;

namespace AssetCache {
 
class CachedItemValue;

class ClientListener
{
public:
    
    virtual void OnAssetClientStateChanged() {};
    virtual void OnAddedToCache(const CacheItemKey &key, bool added) {};
	virtual void OnReceivedFromCache(const CacheItemKey &key, CachedItemValue &&value) {};
};

class Client: public DAVA::TCPChannelListener,
              public Net::AddressRequester
{
public:
    Client();
    
    void AddListener(ClientListener*);
    void RemoveListener(ClientListener*);
    
    bool Connect(const String &ip, uint16 port);
    void Disconnect();

    bool IsConnected();
    
	bool AddToCache(const CacheItemKey &key, const CachedItemValue &value);
    bool RequestFromCache(const CacheItemKey &key);
    bool WarmingUp(const CacheItemKey &key);
    
    //TCPChannelDelegate
    void ChannelOpened(TCPChannel *tcpChannel) override;
    void ChannelClosed(TCPChannel *tcpChannel, const char8* message) override;
    void PacketReceived(DAVA::TCPChannel *tcpChannel, const uint8* packet, size_t length) override;

    // AddressRequester
    virtual void OnAddressResolved() override;
    
    TCPConnection * GetConnection() const;
    
private:
    void OnAddedToCache(KeyedArchive * archieve);
    void OnGetFromCache(KeyedArchive * archieve);

    void StateChanged();
    
private:
    Net::AddressResolver addressResolver;
    std::unique_ptr<TCPConnection> netClient;
    TCPChannel * openedChannel = nullptr;
    
    Set<ClientListener*> listeners;
};

inline TCPConnection * Client::GetConnection() const
{
    return netClient.get();
}
   
inline bool Client::IsConnected()
{
    return (openedChannel != nullptr);
}

    
}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CLIENT_H__

