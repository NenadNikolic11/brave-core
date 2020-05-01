/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

interface GreaselionErrorResponse {
  errorMessage: string
}

interface SavePublisherVisitResponse {
  url: string
  publisherKey: string
  publisherName: string
  mediaKey?: string
  favIconUrl?: string
}

interface GreaselionResponse {
  msgType: 'GreaselionError' | 'SavePublisherVisit'
  mediaType: string
  data: GreaselionErrorResponse | SavePublisherVisitResponse | null
}

interface MediaDurationHandlerRegistrationRequest {
  urlRegex: string
}

interface MediaDurationMetadataResponse {
  url: string
  mediaKey: string
  duration: number
}

interface MediaDurationHandlerRegistrationItem {
  registered: boolean
}

interface MediaDurationHandlerDictionary {
  [index: string]: MediaDurationHandlerRegistrationItem
}

interface GreaselionPortDictionary {
  [index: number]: chrome.runtime.Port
}

let greaselionPorts: GreaselionPortDictionary = {}

let mediaDurationHandlerRegistrations: MediaDurationHandlerDictionary = {}

const handleGreaselionErrorResponse = (tabId: number, mediaType: string, data: GreaselionErrorResponse) => {
  console.error(`Greaselion error: ${data.errorMessage}`)
}

const handleMediaDurationMetadataResponse = (tabId: number, mediaType: string, data: MediaDurationMetadataResponse) => {
  chrome.braveRewards.getMediaPublisherInfo(data.mediaKey, (result: number, info?: RewardsExtension.Publisher) => {
    console.debug(`getMediaPublisherInfo: result=${result}`)

    if (result === 0 && info) {
      console.info('Updating media duration:')
      console.info(`  tabId=${tabId}`)
      console.info(`  url=${data.url}`)
      console.info(`  publisherKey=${info.publisher_key}`)
      console.info(`  publisherName=${info.name}`)
      console.info(`  favIconUrl=${info.favicon_url}`)
      console.info(`  duration=${data.duration}`)

      chrome.braveRewards.updateMediaDuration(
        tabId,
        mediaType,
        data.url,
        info.publisher_key || '',
        info.name || '',
        info.favicon_url || '',
        data.duration)
      return
    }
  })
}

const handleMediaDurationHandlerRegistrationRequest = (tabId: number, mediaType: string, data: MediaDurationHandlerRegistrationRequest) => {
  // If we already registered this media type, exit early
  if (mediaDurationHandlerRegistrations[mediaType] && mediaDurationHandlerRegistrations[mediaType].registered) {
    return
  }

  // Mark this media type as registered
  mediaDurationHandlerRegistrations[mediaType] = {
    registered: true
  }

  chrome.webRequest.onCompleted.addListener(
    // Listener
    function (details) {
      const port = greaselionPorts[tabId]
      if (!port) {
        return
      }
      port.postMessage({
        msgType: 'MediaDurationMetadataRequest',
        url: details.url
      })
    },
    // Filters
    {
      types: [
        'image',
        'media',
        'script',
        'xmlhttprequest'
      ],
      urls: [
        data.urlRegex
      ]
    })
}

const handleSavePublisherVisitResponse = (tabId: number, mediaType: string, data: SavePublisherVisitResponse) => {
  console.info('Visited a publisher url:')
  console.info(`  tabId=${tabId}`)
  console.info(`  mediaType=${mediaType}`)
  console.info(`  url=${data.url}`)
  console.info(`  publisherKey=${data.publisherKey}`)
  console.info(`  publisherName=${data.publisherName}`)
  console.info(`  favIconUrl=${data.favIconUrl}`)

  if (!data.publisherKey) {
    console.error('Failed to handle publisher visit: missing publisher key')
    return
  }

  chrome.braveRewards.getPublisherInfo(data.publisherKey, (result: number, info?: RewardsExtension.Publisher) => {
    console.debug(`getPublisherInfo: result=${result}`)

    if (result === 0 && info) {
      chrome.braveRewards.getPublisherPanelInfo(
        tabId,
        mediaType,
        data.publisherKey)
      return
    }

    // Failed to find publisher info corresponding to this key, so save it now
    if (result === 9) {
      chrome.braveRewards.savePublisherVisit(
        tabId,
        mediaType,
        data.url,
        data.publisherKey,
        data.publisherName,
        data.favIconUrl || '')
      return
    }
  })
}

chrome.runtime.onMessageExternal.addListener((msg, sender, sendResponse) => {
  if (!sender || !sender.tab || !msg) {
    return
  }

  const windowId = sender.tab.windowId
  if (!windowId) {
    return
  }

  const tabId = sender.tab.id
  if (!tabId) {
    return
  }

  const response = msg as GreaselionResponse
  if (!response) {
    console.error('Received invalid response from Greaselion content script')
    return
  }

  if (!response.data) {
    console.error(`Received empty Greaselion response payload for ${msg.msgType} message`)
    return
  }

  switch (msg.msgType) {
    case 'GreaselionError': {
      const data = response.data as GreaselionErrorResponse
      handleGreaselionErrorResponse(tabId, response.mediaType, data)
      break
    }
    case 'SavePublisherVisit': {
      const data = response.data as SavePublisherVisitResponse
      handleSavePublisherVisitResponse(tabId, response.mediaType, data)
      break
    }
  }
})

chrome.runtime.onConnectExternal.addListener(function(port) {
  console.assert(port.name === 'Greaselion')

  if (!port.sender || !port.sender.tab) {
    return
  }

  const tabId = port.sender.tab.id
  if (!tabId) {
    return
  }

  if (greaselionPorts[tabId]) {
    return
  }

  greaselionPorts[tabId] = port

  port.onMessage.addListener(function(msg) {
    switch (msg.msgType) {
      case 'MediaDurationHandlerRegistrationRequest': {
        const data = msg.data as MediaDurationHandlerRegistrationRequest
        handleMediaDurationHandlerRegistrationRequest(tabId, msg.mediaType, data)
        break
      }
      case 'MediaDurationMetadataResponse': {
        const data = msg.data as MediaDurationMetadataResponse
        handleMediaDurationMetadataResponse(tabId, msg.mediaType, data)
        break
      }
    }
  })
})
