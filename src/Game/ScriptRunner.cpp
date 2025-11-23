// ScriptRunner.cpp

#include <iostream>
#include "assets/lua-5.4.6/src/lua.hpp"
#include "src/Game/GameData.hpp"
#include "src/Game/Workspace.hpp"

extern Workspace* global_workspace;  // Workspace.cppで設定してるはず

lua_State* G_L = nullptr;  // ← ここが生き続ける唯一のLuaステート

// 名前実装してからにしよう
// int l_setCubePos(lua_State* L) {
//     const char* name = luaL_checkstring(L, 1);
//     float x = luaL_checknumber(L, 2);
//     float y = luaL_checknumber(L, 3);
//     float z = luaL_checknumber(L, 4);

//     if (!global_workspace) return 0;

//     for (auto& cube : global_workspace->cubes) {
//         if (cube.name == name) {  // ← Cubeにname追加するか、playerならisPlayerで判定
//             cube.pos = Vector3(x, y, z);
//             cube.wakeUp();  // 物理も起こす
//             break;
//         }
//     }
//     return 0;
// }

int l_workspace_getPlayer(lua_State* L) {
    if (!global_workspace) {
        lua_pushnil(L);
        return 1;
    }

    Cube* player = global_workspace->getPlayer();
    if (!player) {
        lua_pushnil(L);
        return 1;
    }

    // Lua側で player.pos.x / y / z にアクセスできる簡易テーブル
    lua_newtable(L);
    lua_pushnumber(L, player->pos.x); lua_setfield(L, -2, "x");
    lua_pushnumber(L, player->pos.y); lua_setfield(L, -2, "y");
    lua_pushnumber(L, player->pos.z); lua_setfield(L, -2, "z");

    return 1; // テーブルを返す
}

void registerWorkspace(lua_State* L) {
    lua_newtable(L); // workspace テーブル

    lua_pushcfunction(L, l_workspace_getPlayer);
    lua_setfield(L, -2, "getPlayer"); // workspace.getPlayer = l_workspace_getPlayer

    lua_setglobal(L, "workspace"); // Lua グローバルに workspace として登録
}
// Luaから呼べる関数
int l_movePlayer(lua_State* L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float z = luaL_checknumber(L, 3);
    if (global_workspace) {
        if (Cube* p = global_workspace->getPlayer()) {
            p->pos = Vector3(x, y, z);
            p->velocity = Vector3(0, 0, 0);
            p->wakeUp();
        }
    }
    return 0;
}

// Connect関数
static int l_connect(lua_State* L) {
    EventBase* event = (EventBase*)lua_touserdata(L, lua_upvalueindex(1));

    // self が渡っている場合は無視して次の引数を関数とする
    int fn_index = 1;
    if (lua_type(L, 1) == LUA_TTABLE) {
        fn_index = 2;
    }

    luaL_checktype(L, fn_index, LUA_TFUNCTION);
    lua_pushvalue(L, fn_index);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    event->connect([ref](std::any value) {
        if (!G_L) return;
        lua_rawgeti(G_L, LUA_REGISTRYINDEX, ref);
        if (value.type() == typeid(float)) {
            lua_pushnumber(G_L, std::any_cast<float>(value));
        } else {
            lua_pushnil(G_L);
        }
        if (lua_pcall(G_L, 1, 0, 0) != LUA_OK) {
            std::cerr << "Lua error: " << lua_tostring(G_L, -1) << std::endl;
            lua_pop(G_L, 1);
        }
    });

    return 0;
}

int initLua() {
    G_L = luaL_newstate();
    luaL_openlibs(G_L);

    // RunService 登録 (Heartbeat のみ)
    lua_newtable(G_L);

    // Heartbeat テーブル
    lua_newtable(G_L);
    lua_pushlightuserdata(G_L, &RunService::Heartbeat);
    lua_pushcclosure(G_L, l_connect, 1);
    lua_setfield(G_L, -2, "Connect"); // Heartbeat.Connect = l_connect

    lua_setfield(G_L, -2, "Heartbeat"); // RunService.Heartbeat = Heartbeat テーブル
    lua_setglobal(G_L, "RunService");

    // Workspace 登録
    registerWorkspace(G_L);

    // グローバル関数
    lua_register(G_L, "movePlayer", l_movePlayer);

    // Steppedはない。
    // lua_pushlightuserdata(G_L, &RunService::Stepped);
    // lua_pushcclosure(G_L, l_connect, 1);
    // lua_setfield(G_L, -2, "Stepped");

    //ファイルパスは絶対にフルで書け！！！！！！！！！！！！！！！！！！......^1024
    if (luaL_dofile(G_L, "src/Game/script/hello.lua") != LUA_OK) {
        std::cerr << "Lua load error: " << lua_tostring(G_L, -1) << std::endl;
        lua_pop(G_L, 1);
    }

    return 0;
}