#pragma once

#include <string>
#include <vector>

struct key_entry_t
{
    int vk;
    std::string ru;
    std::string en;
};

const std::vector<key_entry_t>& GetKeyList();
bool AnyKeyDown();
int PollPressedKey();
const char* KeyName(int key);
