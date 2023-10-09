// SPDX-License-Identifier: MIT

#include <cstdio>
#include <stdexcept>

#include "text_item.hpp"


using namespace std::literals;


TextItem::TextItem(const std::string& key,
                   const std::string& name,
                   const std::string& text) :
    text{text},
    default_text{text}
{
    WUPSConfigCallbacks_t cb;

    cb.getCurrentValueDisplay = [](void* ctx, char* buf, int size) -> int
    {
        if (!ctx)
            return -1;
        auto item = reinterpret_cast<const TextItem*>(ctx);
        return item->getCurrentValueDisplay(buf, size);
    };

    cb.getCurrentValueSelectedDisplay = [](void* ctx, char* buf, int size) -> int
    {
        if (!ctx)
            return -1;
        auto item = reinterpret_cast<const TextItem*>(ctx);
        return item->getCurrentValueSelectedDisplay(buf, size);
    };

    cb.onSelected = [](void* ctx, bool isSelected)
    {
        if (!ctx)
            return;
        auto item = reinterpret_cast<const TextItem*>(ctx);
        item->onSelected(isSelected);
    };

    cb.restoreDefault = [](void* ctx)
    {
        if (!ctx)
            return;
        auto item = reinterpret_cast<TextItem*>(ctx);
        item->restoreDefault();
    };

    cb.isMovementAllowed = [](void* ctx) -> bool
    {
        if (!ctx)
            return true;
        auto item = reinterpret_cast<const TextItem*>(ctx);
        return item->isMovementAllowed();
    };

    cb.callCallback = [](void* ctx) -> bool
    {
        if (!ctx)
            return false;
        auto item = reinterpret_cast<TextItem*>(ctx);
        return item->callback();
    };

    cb.onButtonPressed = [](void* ctx, WUPSConfigButtons button)
    {
        if (!ctx)
            return;
        auto item = reinterpret_cast<TextItem*>(ctx);
        item->onButtonPressed(button);
    };

    cb.onDelete = [](void* ctx)
    {
        if (!ctx)
            return;
        auto item = reinterpret_cast<TextItem*>(ctx);
        item->handle = 0; // don't destroy the item, the parent already destroyed it.
        delete item;
    };


    if (WUPSConfigItem_Create(&handle, key.c_str(), name.c_str(), cb, this) < 0)
        throw std::runtime_error{"could not create config item"};
}


TextItem::~TextItem()
{
    if (handle)
        WUPSConfigItem_Destroy(handle);
}


int
TextItem::getCurrentValueDisplay(char* buf,
                                 std::size_t size)
    const
{
    std::snprintf(buf, size, text.c_str());
    if (size > 3 && text.size() + 1 > size)
        // replace last 3 chars of buf with "..."
        buf[size - 2] = buf[size - 3] = buf[size - 4] = '.';
    return 0;
}


int
TextItem::getCurrentValueSelectedDisplay(char* buf,
                                         std::size_t size)
    const
{
    return getCurrentValueDisplay(buf, size);
}


void
TextItem::onSelected(bool)
    const
{}


void
TextItem::restoreDefault()
{
    text = default_text;
}


bool
TextItem::isMovementAllowed()
    const
{
    return true;
}


bool
TextItem::callback()
{
    return false;
}


void
TextItem::onButtonPressed(WUPSConfigButtons)
{}
