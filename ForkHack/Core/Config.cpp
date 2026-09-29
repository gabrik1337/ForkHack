#include "Core/Config.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <windows.h>

#include "Utils/xorstr.h"

config_t g_cfg;

std::string config_t::ConfigDir()
{
    return "C:\\Astral";
}

namespace
{
    std::string ConfigPath(const std::string& name)
    {
        return std::string(config_t::ConfigDir()) + "\\" + name + ".cfg";
    }

    bool ValidName(const std::string& name)
    {
        if (name.empty() || name.size() > 48)
        {
            return false;
        }

        return name.find_first_of("/\\:\"?<>|") == std::string::npos;
    }

    void WriteBool(std::ostringstream& out, const char* key, bool value)
    {
        out << key << "=" << (value ? 1 : 0) << "\n";
    }

    void WriteInt(std::ostringstream& out, const char* key, int value)
    {
        out << key << "=" << value << "\n";
    }

    void WriteFloat(std::ostringstream& out, const char* key, float value)
    {
        out << key << "=" << value << "\n";
    }

    void WriteColor(std::ostringstream& out, const char* key, c_float_color& color)
    {
        out << key << "=" << color[0] << ' ' << color[1] << ' ' << color[2] << ' ' << color[3] << "\n";
    }

    const std::string& CryptKey()
    {
        static const std::string key = "KDSALDL2DL2LDLASDKKWD2KD2LDLASDLLSA";
        return key;
    }

    void XorCrypt(std::string& data)
    {
        const std::string& key = CryptKey();

        for (size_t i = 0; i < data.size(); i++)
        {
            data[i] ^= key[i % key.size()];
        }
    }

    std::string ToB64(const std::string& data)
    {
        static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((data.size() + 2) / 3) * 4);

        for (size_t i = 0; i < data.size(); i += 3)
        {
            unsigned int n = (unsigned char)data[i] << 16;

            if (i + 1 < data.size())
            {
                n |= (unsigned char)data[i + 1] << 8;
            }

            if (i + 2 < data.size())
            {
                n |= (unsigned char)data[i + 2];
            }

            out += tbl[(n >> 18) & 63];
            out += tbl[(n >> 12) & 63];
            out += (i + 1 < data.size()) ? tbl[(n >> 6) & 63] : '=';
            out += (i + 2 < data.size()) ? tbl[n & 63] : '=';
        }

        return out;
    }

    bool FromB64(const std::string& data, std::string& out)
    {
        auto val = [](char c) -> int
        {
            if (c >= 'A' && c <= 'Z') return c - 'A';
            if (c >= 'a' && c <= 'z') return c - 'a' + 26;
            if (c >= '0' && c <= '9') return c - '0' + 52;
            if (c == '+') return 62;
            if (c == '/') return 63;
            return -1;
        };

        std::string clean;

        for (char c : data)
        {
            if (c == '\r' || c == '\n' || c == ' ' || c == '\t')
            {
                continue;
            }

            clean += c;
        }

        if (clean.empty() || clean.size() % 4 != 0)
        {
            return false;
        }

        out.clear();
        out.reserve((clean.size() / 4) * 3);

        for (size_t i = 0; i < clean.size(); i += 4)
        {
            int a = val(clean[i]);
            int b = val(clean[i + 1]);
            int c = (clean[i + 2] == '=') ? 0 : val(clean[i + 2]);
            int d = (clean[i + 3] == '=') ? 0 : val(clean[i + 3]);

            if (a < 0 || b < 0 || c < 0 || d < 0)
            {
                return false;
            }

            if ((clean[i + 2] == '=' && clean[i + 3] != '=') || (i + 4 != clean.size() && (clean[i + 2] == '=' || clean[i + 3] == '=')))
            {
                return false;
            }

            unsigned int n = (a << 18) | (b << 12) | (c << 6) | d;
            out += (char)((n >> 16) & 255);
            if (clean[i + 2] != '=') out += (char)((n >> 8) & 255);
            if (clean[i + 3] != '=') out += (char)(n & 255);
        }

        return true;
    }

    bool ReadBool(const std::string& value, bool& out)
    {
        std::istringstream stream(value);
        int temp = 0;

        if (!(stream >> temp))
        {
            return false;
        }

        out = temp != 0;
        return true;
    }

    bool ReadInt(const std::string& value, int& out)
    {
        std::istringstream stream(value);
        return !!(stream >> out);
    }

    bool ReadFloat(const std::string& value, float& out)
    {
        std::istringstream stream(value);
        return !!(stream >> out);
    }

    bool ReadColor(const std::string& value, c_float_color& out)
    {
        std::istringstream stream(value);
        return !!(stream >> out[0] >> out[1] >> out[2] >> out[3]);
    }
}

bool config_t::Save(const std::string& name)
{
    if (!ValidName(name))
    {
        return false;
    }

    CreateDirectoryA(ConfigDir().c_str(), nullptr);

    std::ostringstream body;

    WriteColor(body, "accent", accent);
    WriteInt(body, "ui_scale", ui_scale);
    WriteInt(body, "language", language);
    WriteBool(body, "showbinds", showbinds);
    WriteFloat(body, "binds_x", binds_x);
    WriteFloat(body, "binds_y", binds_y);
    WriteBool(body, "godmode", godmode);
    WriteInt(body, "godmode_key", godmode_bind.key);
    WriteInt(body, "godmode_mode", godmode_bind.mode);
    WriteBool(body, "nofall", nofall);
    WriteInt(body, "nofall_key", nofall_bind.key);
    WriteInt(body, "nofall_mode", nofall_bind.mode);
    WriteBool(body, "fastbeg", fastbeg);
    WriteInt(body, "fastbeg_key", fastbeg_bind.key);
    WriteInt(body, "fastbeg_mode", fastbeg_bind.mode);
    WriteFloat(body, "fastbegs", fastbegs);
    WriteBool(body, "wh", wh);
    WriteInt(body, "wh_key", wh_bind.key);
    WriteInt(body, "wh_mode", wh_bind.mode);
    WriteFloat(body, "whDistance", whDistance);
    WriteBool(body, "speedhack", speedhack);
    WriteInt(body, "speedhack_key", speedhack_bind.key);
    WriteInt(body, "speedhack_mode", speedhack_bind.mode);
    WriteFloat(body, "MaxSpd", MaxSpd);
    WriteBool(body, "airbreake", airbreake);
    WriteInt(body, "airbreake_key", airbreake_bind.key);
    WriteInt(body, "airbreake_mode", airbreake_bind.mode);
    WriteBool(body, "changemodel", changemodel);
    WriteInt(body, "changemodel_key", changemodel_bind.key);
    WriteInt(body, "changemodel_mode", changemodel_bind.mode);
    WriteInt(body, "playerSkinID", playerSkinID);
    WriteBool(body, "nightmode", nightmode);
    WriteInt(body, "nightmode_key", nightmode_bind.key);
    WriteInt(body, "nightmode_mode", nightmode_bind.mode);
    WriteBool(body, "customtime", customtime);
    WriteInt(body, "customtime_key", customtime_bind.key);
    WriteInt(body, "customtime_mode", customtime_bind.mode);
    WriteFloat(body, "timehour", timehour);
    WriteBool(body, "freezetime", freezetime);
    WriteInt(body, "freezetime_key", freezetime_bind.key);
    WriteInt(body, "freezetime_mode", freezetime_bind.mode);
    WriteInt(body, "weather", weather);
    WriteBool(body, "customcolor", customcolor);
    WriteInt(body, "customcolor_key", customcolor_bind.key);
    WriteInt(body, "customcolor_mode", customcolor_bind.mode);
    WriteBool(body, "fullbright", fullbright);
    WriteInt(body, "fullbright_key", fullbright_bind.key);
    WriteInt(body, "fullbright_mode", fullbright_bind.mode);
    WriteBool(body, "skychange", skychange);
    WriteInt(body, "skychange_key", skychange_bind.key);
    WriteInt(body, "skychange_mode", skychange_bind.mode);
    WriteColor(body, "skycol", skycol);
    WriteColor(body, "skybotcol", skybotcol);
    WriteColor(body, "ambcol", ambcol);
    WriteColor(body, "watercol", watercol);
    WriteBool(body, "fogchange", fogchange);
    WriteInt(body, "fogchange_key", fogchange_bind.key);
    WriteInt(body, "fogchange_mode", fogchange_bind.mode);
    WriteFloat(body, "fogval", fogval);
    WriteBool(body, "suncolor", suncolor);
    WriteInt(body, "suncolor_key", suncolor_bind.key);
    WriteInt(body, "suncolor_mode", suncolor_bind.mode);
    WriteColor(body, "suncol", suncol);
    WriteBool(body, "removals", removals);
    WriteInt(body, "removals_key", removals_bind.key);
    WriteInt(body, "removals_mode", removals_bind.mode);
    WriteInt(body, "removal_flags", (int)removal_flags);
    WriteBool(body, "carcolor", carcolor);
    WriteInt(body, "carcolor_key", carcolor_bind.key);
    WriteInt(body, "carcolor_mode", carcolor_bind.mode);
    WriteColor(body, "carrgb1", carrgb1);
    WriteBool(body, "rapidfire", rapidfire);
    WriteInt(body, "rapidfire_key", rapidfire_bind.key);
    WriteInt(body, "rapidfire_mode", rapidfire_bind.mode);
    WriteBool(body, "infammo", infammo);
    WriteInt(body, "infammo_key", infammo_bind.key);
    WriteInt(body, "infammo_mode", infammo_bind.mode);

    WriteInt(body, "wh_flags", (int)wh_flags);
    WriteColor(body, "whcol", whcol);
    WriteColor(body, "hpcol", hpcol);
    WriteColor(body, "armorcol", armorcol);
    WriteColor(body, "distcol", distcol);
    WriteColor(body, "skelcol", skelcol);
    WriteColor(body, "snapcol", snapcol);
    WriteBool(body, "gamespeed", gamespeed);
    WriteInt(body, "gamespeed_key", gamespeed_bind.key);
    WriteInt(body, "gamespeed_mode", gamespeed_bind.mode);
    WriteFloat(body, "gamespeedval", gamespeedval);
    WriteBool(body, "fastrot", fastrot);
    WriteInt(body, "fastrot_key", fastrot_bind.key);
    WriteInt(body, "fastrot_mode", fastrot_bind.mode);
    WriteBool(body, "nobikefall", nobikefall);
    WriteInt(body, "nobikefall_key", nobikefall_bind.key);
    WriteInt(body, "nobikefall_mode", nobikefall_bind.mode);
    WriteBool(body, "waterdrive", waterdrive);
    WriteInt(body, "waterdrive_key", waterdrive_bind.key);
    WriteInt(body, "waterdrive_mode", waterdrive_bind.mode);
    WriteBool(body, "carfly", carfly);
    WriteInt(body, "carfly_key", carfly_bind.key);
    WriteInt(body, "carfly_mode", carfly_bind.mode);
    WriteBool(body, "autoengine", autoengine);
    WriteInt(body, "autoengine_key", autoengine_bind.key);
    WriteInt(body, "autoengine_mode", autoengine_bind.mode);
    WriteBool(body, "autounlock", autounlock);
    WriteInt(body, "autounlock_key", autounlock_bind.key);
    WriteInt(body, "autounlock_mode", autounlock_bind.mode);
    WriteBool(body, "fastcross", fastcross);
    WriteInt(body, "fastcross_key", fastcross_bind.key);
    WriteInt(body, "fastcross_mode", fastcross_bind.mode);
    WriteBool(body, "norecoil", norecoil);
    WriteInt(body, "norecoil_key", norecoil_bind.key);
    WriteInt(body, "norecoil_mode", norecoil_bind.mode);
    WriteBool(body, "nospread", nospread);
    WriteInt(body, "nospread_key", nospread_bind.key);
    WriteInt(body, "nospread_mode", nospread_bind.mode);
    WriteBool(body, "ram", ram);
    WriteInt(body, "ram_key", ram_bind.key);
    WriteInt(body, "ram_mode", ram_bind.mode);
    WriteFloat(body, "rampower", rampower);
    WriteBool(body, "trigger", trigger);
    WriteInt(body, "trigger_key", trigger_bind.key);
    WriteInt(body, "trigger_mode", trigger_bind.mode);
    WriteFloat(body, "triggerdelay", triggerdelay);
    WriteBool(body, "nocamcol", nocamcol);
    WriteInt(body, "nocamcol_key", nocamcol_bind.key);
    WriteInt(body, "nocamcol_mode", nocamcol_bind.mode);
    WriteBool(body, "aspect", aspect);
    WriteInt(body, "aspect_key", aspect_bind.key);
    WriteInt(body, "aspect_mode", aspect_bind.mode);
    WriteFloat(body, "aspectval", aspectval);
    WriteBool(body, "fov", fov);
    WriteInt(body, "fov_key", fov_bind.key);
    WriteInt(body, "fov_mode", fov_bind.mode);
    WriteFloat(body, "fovval", fovval);
    WriteBool(body, "camhack", camhack);
    WriteInt(body, "camhack_key", camhack_bind.key);
    WriteInt(body, "camhack_mode", camhack_bind.mode);
    WriteFloat(body, "camhackspeed", camhackspeed);
    WriteBool(body, "camhackteleport", camhackteleport);
    WriteBool(body, "nocol", nocol);
    WriteInt(body, "nocol_key", nocol_bind.key);
    WriteInt(body, "nocol_mode", nocol_bind.mode);
    WriteInt(body, "nocol_flags", (int)nocol_flags);

    std::string raw = body.str();
    XorCrypt(raw);

    std::ofstream file(ConfigPath(name), std::ios::trunc);

    if (!file.is_open())
    {
        return false;
    }

    file << ToB64(raw);
    return !!file;
}

bool config_t::Load(const std::string& name)
{
    if (!ValidName(name))
    {
        return false;
    }

    std::ifstream file(ConfigPath(name));

    if (!file.is_open())
    {
        return false;
    }

    std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string decoded;

    if (!FromB64(raw, decoded))
    {
        return false;
    }

    XorCrypt(decoded);
    raw.swap(decoded);

    std::istringstream stream(raw);
    std::string line;

    while (std::getline(stream, line))
    {
        size_t pos = line.find('=');

        if (pos == std::string::npos)
        {
            continue;
        }

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "accent") { ReadColor(value, accent); }
        else if (key == "godmode") { ReadBool(value, godmode); }
        else if (key == "godmode_key") { ReadInt(value, godmode_bind.key); }
        else if (key == "godmode_mode") { ReadInt(value, godmode_bind.mode); }
        else if (key == "nofall") { ReadBool(value, nofall); }
        else if (key == "nofall_key") { ReadInt(value, nofall_bind.key); }
        else if (key == "nofall_mode") { ReadInt(value, nofall_bind.mode); }
        else if (key == "fastbeg") { ReadBool(value, fastbeg); }
        else if (key == "fastbeg_key") { ReadInt(value, fastbeg_bind.key); }
        else if (key == "fastbeg_mode") { ReadInt(value, fastbeg_bind.mode); }
        else if (key == "fastbegs") { ReadFloat(value, fastbegs); }
        else if (key == "wh") { ReadBool(value, wh); }
        else if (key == "wh_key") { ReadInt(value, wh_bind.key); }
        else if (key == "wh_mode") { ReadInt(value, wh_bind.mode); }
        else if (key == "whDistance") { ReadFloat(value, whDistance); }
        else if (key == "speedhack") { ReadBool(value, speedhack); }
        else if (key == "speedhack_key") { ReadInt(value, speedhack_bind.key); }
        else if (key == "speedhack_mode") { ReadInt(value, speedhack_bind.mode); }
        else if (key == "MaxSpd") { ReadFloat(value, MaxSpd); }
        else if (key == "airbreake") { ReadBool(value, airbreake); }
        else if (key == "airbreake_key") { ReadInt(value, airbreake_bind.key); }
        else if (key == "airbreake_mode") { ReadInt(value, airbreake_bind.mode); }
        else if (key == "changemodel") { ReadBool(value, changemodel); }
        else if (key == "changemodel_key") { ReadInt(value, changemodel_bind.key); }
        else if (key == "changemodel_mode") { ReadInt(value, changemodel_bind.mode); }
        else if (key == "playerSkinID") { ReadInt(value, playerSkinID); }
        else if (key == "nightmode") { ReadBool(value, nightmode); }
        else if (key == "nightmode_key") { ReadInt(value, nightmode_bind.key); }
        else if (key == "nightmode_mode") { ReadInt(value, nightmode_bind.mode); }
        else if (key == "customtime") { ReadBool(value, customtime); }
        else if (key == "customtime_key") { ReadInt(value, customtime_bind.key); }
        else if (key == "customtime_mode") { ReadInt(value, customtime_bind.mode); }
        else if (key == "timehour") { ReadFloat(value, timehour); }
        else if (key == "freezetime") { ReadBool(value, freezetime); }
        else if (key == "freezetime_key") { ReadInt(value, freezetime_bind.key); }
        else if (key == "freezetime_mode") { ReadInt(value, freezetime_bind.mode); }
        else if (key == "weather") { ReadInt(value, weather); }
        else if (key == "customcolor") { ReadBool(value, customcolor); }
        else if (key == "customcolor_key") { ReadInt(value, customcolor_bind.key); }
        else if (key == "customcolor_mode") { ReadInt(value, customcolor_bind.mode); }
        else if (key == "fullbright") { ReadBool(value, fullbright); }
        else if (key == "fullbright_key") { ReadInt(value, fullbright_bind.key); }
        else if (key == "fullbright_mode") { ReadInt(value, fullbright_bind.mode); }
        else if (key == "skychange") { ReadBool(value, skychange); }
        else if (key == "skychange_key") { ReadInt(value, skychange_bind.key); }
        else if (key == "skychange_mode") { ReadInt(value, skychange_bind.mode); }
        else if (key == "skycol") { ReadColor(value, skycol); }
        else if (key == "skybotcol") { ReadColor(value, skybotcol); }
        else { LoadTail(key, value); }
    }

    if (ui_scale < 50)
    {
        ui_scale = 50;
    }
    else if (ui_scale > 300)
    {
        ui_scale = 300;
    }

    if (language < 0 || language > 1)
    {
        language = 0;
    }

    if (weather < 0 || weather > 20)
    {
        weather = 0;
    }

    return true;
}

void config_t::LoadTail(const std::string& key, const std::string& value)
{
    if (key == "ambcol") { ReadColor(value, ambcol); }
    else if (key == "watercol") { ReadColor(value, watercol); }
    else if (key == "fogchange") { ReadBool(value, fogchange); }
    else if (key == "fogchange_key") { ReadInt(value, fogchange_bind.key); }
    else if (key == "fogchange_mode") { ReadInt(value, fogchange_bind.mode); }
    else if (key == "fogval") { ReadFloat(value, fogval); }
    else if (key == "suncolor") { ReadBool(value, suncolor); }
    else if (key == "suncolor_key") { ReadInt(value, suncolor_bind.key); }
    else if (key == "suncolor_mode") { ReadInt(value, suncolor_bind.mode); }
    else if (key == "suncol") { ReadColor(value, suncol); }
    else if (key == "removals") { ReadBool(value, removals); }
    else if (key == "removals_key") { ReadInt(value, removals_bind.key); }
    else if (key == "removals_mode") { ReadInt(value, removals_bind.mode); }
    else if (key == "removal_flags") { int t = 0; if (ReadInt(value, t)) { removal_flags = (unsigned int)t; } }
    else if (key == "carcolor") { ReadBool(value, carcolor); }
    else if (key == "carcolor_key") { ReadInt(value, carcolor_bind.key); }
    else if (key == "carcolor_mode") { ReadInt(value, carcolor_bind.mode); }
    else if (key == "carrgb1") { ReadColor(value, carrgb1); }
    else if (key == "rapidfire") { ReadBool(value, rapidfire); }
    else if (key == "rapidfire_key") { ReadInt(value, rapidfire_bind.key); }
    else if (key == "rapidfire_mode") { ReadInt(value, rapidfire_bind.mode); }
    else if (key == "infammo") { ReadBool(value, infammo); }
    else if (key == "infammo_key") { ReadInt(value, infammo_bind.key); }
    else if (key == "infammo_mode") { ReadInt(value, infammo_bind.mode); }

    else if (key == "wh_flags") { int t = 15; if (ReadInt(value, t)) { wh_flags = (unsigned int)t; } }
    else if (key == "whcol") { ReadColor(value, whcol); }
    else if (key == "hpcol") { ReadColor(value, hpcol); }
    else if (key == "armorcol") { ReadColor(value, armorcol); }
    else if (key == "distcol") { ReadColor(value, distcol); }
    else if (key == "skelcol") { ReadColor(value, skelcol); }
    else if (key == "snapcol") { ReadColor(value, snapcol); }
    else if (key == "gamespeed") { ReadBool(value, gamespeed); }
    else if (key == "gamespeed_key") { ReadInt(value, gamespeed_bind.key); }
    else if (key == "gamespeed_mode") { ReadInt(value, gamespeed_bind.mode); }
    else if (key == "gamespeedval") { ReadFloat(value, gamespeedval); }
        else if (key == "fastrot") { ReadBool(value, fastrot); }
        else if (key == "fastrot_key") { ReadInt(value, fastrot_bind.key); }
        else if (key == "fastrot_mode") { ReadInt(value, fastrot_bind.mode); }
    else if (key == "nobikefall") { ReadBool(value, nobikefall); }
    else if (key == "nobikefall_key") { ReadInt(value, nobikefall_bind.key); }
    else if (key == "nobikefall_mode") { ReadInt(value, nobikefall_bind.mode); }
    else if (key == "waterdrive") { ReadBool(value, waterdrive); }
    else if (key == "waterdrive_key") { ReadInt(value, waterdrive_bind.key); }
    else if (key == "waterdrive_mode") { ReadInt(value, waterdrive_bind.mode); }
    else if (key == "carfly") { ReadBool(value, carfly); }
    else if (key == "carfly_key") { ReadInt(value, carfly_bind.key); }
        else if (key == "carfly_mode") { ReadInt(value, carfly_bind.mode); }
    else if (key == "autoengine") { ReadBool(value, autoengine); }
    else if (key == "autoengine_key") { ReadInt(value, autoengine_bind.key); }
    else if (key == "autoengine_mode") { ReadInt(value, autoengine_bind.mode); }
    else if (key == "autounlock") { ReadBool(value, autounlock); }
    else if (key == "autounlock_key") { ReadInt(value, autounlock_bind.key); }
    else if (key == "autounlock_mode") { ReadInt(value, autounlock_bind.mode); }
    else if (key == "fastcross") { ReadBool(value, fastcross); }
    else if (key == "fastcross_key") { ReadInt(value, fastcross_bind.key); }
    else if (key == "fastcross_mode") { ReadInt(value, fastcross_bind.mode); }
    else if (key == "norecoil") { ReadBool(value, norecoil); }
    else if (key == "norecoil_key") { ReadInt(value, norecoil_bind.key); }
    else if (key == "norecoil_mode") { ReadInt(value, norecoil_bind.mode); }
    else if (key == "nospread") { ReadBool(value, nospread); }
    else if (key == "nospread_key") { ReadInt(value, nospread_bind.key); }
    else if (key == "nospread_mode") { ReadInt(value, nospread_bind.mode); }
    else if (key == "ram") { ReadBool(value, ram); }
    else if (key == "ram_key") { ReadInt(value, ram_bind.key); }
    else if (key == "ram_mode") { ReadInt(value, ram_bind.mode); }
    else if (key == "rampower") { ReadFloat(value, rampower); }
    else if (key == "trigger") { ReadBool(value, trigger); }
    else if (key == "trigger_key") { ReadInt(value, trigger_bind.key); }
    else if (key == "trigger_mode") { ReadInt(value, trigger_bind.mode); }
    else if (key == "triggerdelay") { ReadFloat(value, triggerdelay); }
    else if (key == "nocamcol") { ReadBool(value, nocamcol); }
    else if (key == "nocamcol_key") { ReadInt(value, nocamcol_bind.key); }
    else if (key == "nocamcol_mode") { ReadInt(value, nocamcol_bind.mode); }
    else if (key == "aspect") { ReadBool(value, aspect); }
    else if (key == "aspect_key") { ReadInt(value, aspect_bind.key); }
    else if (key == "aspect_mode") { ReadInt(value, aspect_bind.mode); }
    else if (key == "aspectval") { ReadFloat(value, aspectval); }
    else if (key == "fov") { ReadBool(value, fov); }
    else if (key == "fov_key") { ReadInt(value, fov_bind.key); }
    else if (key == "fov_mode") { ReadInt(value, fov_bind.mode); }
    else if (key == "fovval") { ReadFloat(value, fovval); }
    else if (key == "camhack") { ReadBool(value, camhack); }
    else if (key == "camhack_key") { ReadInt(value, camhack_bind.key); }
    else if (key == "camhack_mode") { ReadInt(value, camhack_bind.mode); }
    else if (key == "camhackspeed") { ReadFloat(value, camhackspeed); }
    else if (key == "camhackteleport") { ReadBool(value, camhackteleport); }
    else if (key == "nocol") { ReadBool(value, nocol); }
    else if (key == "nocol_key") { ReadInt(value, nocol_bind.key); }
    else if (key == "nocol_mode") { ReadInt(value, nocol_bind.mode); }
    else if (key == "nocol_flags") { int t = 7; if (ReadInt(value, t)) { nocol_flags = (unsigned int)t; } }
    else if (key == "ui_scale") { ReadInt(value, ui_scale); }
    else if (key == "language") { ReadInt(value, language); }
    else if (key == "showbinds") { ReadBool(value, showbinds); }
    else if (key == "binds_x") { ReadFloat(value, binds_x); }
    else if (key == "binds_y") { ReadFloat(value, binds_y); }
}

bool config_t::Remove(const std::string& name)
{
    if (!ValidName(name))
    {
        return false;
    }

    std::error_code ec;
    std::filesystem::remove(ConfigPath(name), ec);

    auto& list = g_cfg.cfg_list;
    list.erase(std::remove(list.begin(), list.end(), name), list.end());

    if (g_cfg.cfg_selected >= (int)list.size())
    {
        g_cfg.cfg_selected = 0;
    }

    return !ec;
}

void config_t::EnsureDir()
{
    CreateDirectoryA(ConfigDir().c_str(), nullptr);
}

void config_t::RefreshList()
{
    g_cfg.cfg_list.clear();
    g_cfg.cfg_selected = 0;

    std::error_code ec;

    if (!std::filesystem::exists(ConfigDir(), ec))
    {
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(ConfigDir(), ec))
    {
        if (!entry.is_regular_file(ec))
        {
            continue;
        }

        if (entry.path().extension() != ".cfg")
        {
            continue;
        }

        std::string name = entry.path().stem().string();

        if (ValidName(name))
        {
            g_cfg.cfg_list.push_back(name);
        }
    }
}

void SaveGeneralConfig()
{
    if (g_cfg.cfg_list.empty())
    {
        return;
    }

    int sel = g_cfg.cfg_selected;

    if (sel < 0 || sel >= (int)g_cfg.cfg_list.size())
    {
        sel = 0;
    }

    g_cfg.Save(g_cfg.cfg_list[sel]);
}
