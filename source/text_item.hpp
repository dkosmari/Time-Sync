// SPDX-License-Identifier: MIT

#ifndef TEXT_ITEM_HPP
#define TEXT_ITEM_HPP

#include <string>

#include <wups.h>


struct TextItem {
    WUPSConfigItemHandle handle = 0;
    std::string text;
    std::string default_text;


    TextItem(const std::string& key,
             const std::string& name,
             const std::string& text);

    // disallow moving, since the callbacks store the `this` pointer.
    TextItem(TextItem&&) = delete;

    virtual ~TextItem();


    virtual int getCurrentValueDisplay(char* buf, std::size_t size) const;

    virtual int getCurrentValueSelectedDisplay(char* buf, std::size_t size) const;

    virtual void onSelected(bool isSelected) const;

    virtual void restoreDefault();

    virtual bool isMovementAllowed() const;

    virtual bool callback();

    virtual void onButtonPressed(WUPSConfigButtons button);

};


#endif
