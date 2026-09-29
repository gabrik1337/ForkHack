#pragma once

#include <string>
#include <vector>

#include "Core/Color.hpp"

struct keybind_t
{
    int key = -1;
    int mode = 0;
    bool manual = false;
};

struct config_t
{
    bool menu_open = false;
    c_float_color accent = c_float_color(168, 168, 255);
    int ui_scale = 100;
    int language = 0;
    bool showbinds = false;
    float binds_x = 10.0f;
    float binds_y = -1.0f;
    bool godmode = false;
    keybind_t godmode_bind{};
    bool nofall = false;
    keybind_t nofall_bind{};
    bool fastbeg = false;
    keybind_t fastbeg_bind{};
    float fastbegs = 1.0f;
    bool wh = false;
    keybind_t wh_bind{};
    float whDistance = 100.0f;
    bool speedhack = false;
    keybind_t speedhack_bind{};
    float MaxSpd = 5.0f;
    bool airbreake = false;
    keybind_t airbreake_bind{};
    bool changemodel = false;
    keybind_t changemodel_bind{};
    int playerSkinID = 0;
    bool nightmode = false;
    keybind_t nightmode_bind{};
    bool customtime = false;
    keybind_t customtime_bind{};
    float timehour = 12.0f;
    bool freezetime = false;
    keybind_t freezetime_bind{};
    int weather = 0;
    bool customcolor = false;
    keybind_t customcolor_bind{};
    bool fullbright = false;
    keybind_t fullbright_bind{};
    bool skychange = false;
    keybind_t skychange_bind{};
    c_float_color skycol = c_float_color(90, 140, 200);
    c_float_color skybotcol = c_float_color(140, 180, 215);
    c_float_color ambcol = c_float_color(130, 130, 140);
    c_float_color watercol = c_float_color(50, 120, 170);
    bool fogchange = false;
    keybind_t fogchange_bind{};
    float fogval = 150.0f;
    bool suncolor = false;
    keybind_t suncolor_bind{};
    c_float_color suncol = c_float_color(255, 200, 100);
    bool removals = false;
    keybind_t removals_bind{};
    unsigned int removal_flags = 0;
    bool carcolor = false;
    keybind_t carcolor_bind{};
    c_float_color carrgb1 = c_float_color(255, 255, 255);
    bool rapidfire = false;
    keybind_t rapidfire_bind{};
    bool infammo = false;
    keybind_t infammo_bind{};
    unsigned int wh_flags = 63;
    c_float_color whcol = c_float_color(168, 168, 255);
    c_float_color hpcol = c_float_color(110, 220, 110);
    c_float_color armorcol = c_float_color(110, 150, 255);
    c_float_color distcol = c_float_color(255, 255, 255);
    c_float_color skelcol = c_float_color(255, 255, 255);
    c_float_color snapcol = c_float_color(0, 255, 0);
    bool gamespeed = false;
    keybind_t gamespeed_bind{};
    float gamespeedval = 1.0f;
    bool fastrot = false;
    keybind_t fastrot_bind{};
    bool nobikefall = false;
    keybind_t nobikefall_bind{};
    bool waterdrive = false;
    keybind_t waterdrive_bind{};
    bool carfly = false;
    keybind_t carfly_bind{};
    bool autoengine = false;
    keybind_t autoengine_bind{};
    bool autounlock = false;
    keybind_t autounlock_bind{};
    bool fastcross = false;
    keybind_t fastcross_bind{};
    bool norecoil = false;
    keybind_t norecoil_bind{};
    bool nospread = false;
    keybind_t nospread_bind{};
    bool ram = false;
    keybind_t ram_bind{};
    float rampower = 8.0f;
    bool trigger = false;
    keybind_t trigger_bind{};
    float triggerdelay = 0.0f;
    bool nocamcol = false;
    keybind_t nocamcol_bind{};
    bool aspect = false;
    keybind_t aspect_bind{};
    float aspectval = 1.78f;
    bool fov = false;
    keybind_t fov_bind{};
    float fovval = 70.0f;
    bool camhack = false;
    keybind_t camhack_bind{};
    float camhackspeed = 0.05f;
    bool camhackteleport = true;
    bool nocol = false;
    keybind_t nocol_bind{};
    unsigned int nocol_flags = 7;
    int cfg_selected = 0;
    char cfg_search[64]{};
    char cfg_name[64]{};
    std::vector<std::string> cfg_list{};

    bool Save(const std::string& name);
    bool Load(const std::string& name);
    void LoadTail(const std::string& key, const std::string& value);
    static bool Remove(const std::string& name);
    static void RefreshList();
    static void EnsureDir();
    static std::string ConfigDir();
};

void SaveGeneralConfig();

extern config_t g_cfg;
