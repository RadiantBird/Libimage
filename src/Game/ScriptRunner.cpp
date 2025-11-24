// src/Game/ScriptRunner.cpp

#include <iostream>
#include "assets/lua-5.4.6/src/lua.hpp"
#include "src/Game/GameData.hpp"
#include "src/Game/Workspace.hpp"
#include "src/Game/Instance.hpp"

extern Workspace* global_workspace;
lua_State* G_L = nullptr;

// ===================================================================
// Lua バインディング: Instance
// ===================================================================
// 前方定義
void wrapInstance(lua_State* L, Instance* inst);

// Instance* を Lua の lightuserdata として push
void pushInstance(lua_State* L, Instance* inst) {
    if (!inst) {
        lua_pushnil(L);
        return;
    }
    lua_pushlightuserdata(L, inst);
}

// Lua から Instance* を取得
Instance* toInstance(lua_State* L, int index) {
    return (Instance*)lua_touserdata(L, index);
}

// workspace:FindFirstChild(name)
int l_workspace_FindFirstChild(lua_State* L) {
    // self (workspace テーブル) を無視して、第2引数から取得
    int nameIndex = 1;
    if (lua_istable(L, 1)) {
        nameIndex = 2; // : で呼ばれた場合、第1引数は self
    }
    
    const char* name = luaL_checkstring(L, nameIndex);
    
    if (!global_workspace) {
        lua_pushnil(L);
        return 1;
    }

    Instance* found = global_workspace->FindFirstChild(name);
    
    // Instance をラップして返す
    if (found && found->IsA("Part")) {
        wrapInstance(L, found);
    } else {
        lua_pushnil(L);
    }
    
    return 1;
}

// workspace:GetChildren()
int l_workspace_GetChildren(lua_State* L) {
    // self を無視
    if (!global_workspace) {
        lua_newtable(L);
        return 1;
    }

    lua_newtable(L);
    int index = 1;
    
    // cubes配列を返す
    for (auto& cube : global_workspace->cubes) {
        lua_pushinteger(L, index);
        wrapInstance(L, &cube);
        lua_settable(L, -3);
        index++;
    }
    
    return 1;
}

// workspace:getPlayer() (既存)
int l_workspace_getPlayer(lua_State* L) {
    // self を無視
    if (!global_workspace) {
        lua_pushnil(L);
        return 1;
    }

    Cube* player = global_workspace->getPlayer();
    if (!player) {
        lua_pushnil(L);
        return 1;
    }

    // プレイヤーを Instance としてラップして返す
    wrapInstance(L, player);
    return 1;
}

// ===================================================================
// Lua バインディング: Part (Cube)
// ===================================================================

// Part メタテーブルの作成
void createPartMetatable(lua_State* L) {
    lua_newtable(L); // メタテーブル
    
    // __index メタメソッド（プロパティアクセス時）
    lua_pushcfunction(L, [](lua_State* L) -> int {
        // L[1] = self (Part テーブル)
        // L[2] = key (プロパティ名 or メソッド名)
        
        const char* key = luaL_checkstring(L, 2);
        
        // _ptr から Instance ポインタを取得
        lua_getfield(L, 1, "_ptr");
        Instance* inst = (Instance*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        
        if (!inst) {
            lua_pushnil(L);
            return 1;
        }
        
        // プロパティアクセス
        if (strcmp(key, "Name") == 0) {
            lua_pushstring(L, inst->Name.c_str());
            return 1;
        }
        else if (strcmp(key, "ClassName") == 0) {
            lua_pushstring(L, inst->ClassName.c_str());
            return 1;
        }
        else if (strcmp(key, "Position") == 0 && inst->IsA("Part")) {
            Cube* cube = static_cast<Cube*>(inst);
            lua_newtable(L);
            lua_pushnumber(L, cube->pos.x); lua_setfield(L, -2, "X");
            lua_pushnumber(L, cube->pos.y); lua_setfield(L, -2, "Y");
            lua_pushnumber(L, cube->pos.z); lua_setfield(L, -2, "Z");
            return 1;
        }
        // メソッド: FindFirstChild
        else if (strcmp(key, "FindFirstChild") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                // L[1] = self
                // L[2] = name
                const char* name = luaL_checkstring(L, 2);
                
                lua_getfield(L, 1, "_ptr");
                Instance* inst = (Instance*)lua_touserdata(L, -1);
                lua_pop(L, 1);
                
                if (!inst) {
                    lua_pushnil(L);
                    return 1;
                }
                
                Instance* found = inst->FindFirstChild(name);
                if (found) {
                    wrapInstance(L, found);
                } else {
                    lua_pushnil(L);
                }
                return 1;
            });
            return 1;
        }
        // メソッド: GetChildren
        else if (strcmp(key, "GetChildren") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                // L[1] = self
                lua_getfield(L, 1, "_ptr");
                Instance* inst = (Instance*)lua_touserdata(L, -1);
                lua_pop(L, 1);
                
                if (!inst) {
                    lua_newtable(L);
                    return 1;
                }
                
                lua_newtable(L);
                int index = 1;
                
                for (auto* child : inst->GetChildren()) {
                    lua_pushinteger(L, index);
                    wrapInstance(L, child);
                    lua_settable(L, -3);
                    index++;
                }
                
                return 1;
            });
            return 1;
        }
        // メソッド: IsA
        else if (strcmp(key, "IsA") == 0) {
            lua_pushcfunction(L, [](lua_State* L) -> int {
                // L[1] = self
                // L[2] = className
                const char* className = luaL_checkstring(L, 2);
                
                lua_getfield(L, 1, "_ptr");
                Instance* inst = (Instance*)lua_touserdata(L, -1);
                lua_pop(L, 1);
                
                if (!inst) {
                    lua_pushboolean(L, false);
                    return 1;
                }
                
                lua_pushboolean(L, inst->IsA(className));
                return 1;
            });
            return 1;
        }
        
        lua_pushnil(L);
        return 1;
    });
    lua_setfield(L, -2, "__index");
    
    // __newindex メタメソッド（プロパティ設定時）
    lua_pushcfunction(L, [](lua_State* L) -> int {
        // L[1] = self
        // L[2] = key
        // L[3] = value
        
        const char* key = luaL_checkstring(L, 2);
        
        lua_getfield(L, 1, "_ptr");
        Instance* inst = (Instance*)lua_touserdata(L, -1);
        lua_pop(L, 1);
        
        if (!inst || !inst->IsA("Part")) {
            return 0;
        }
        
        Cube* cube = static_cast<Cube*>(inst);
        
        if (strcmp(key, "Position") == 0 && lua_istable(L, 3)) {
            lua_getfield(L, 3, "X");
            lua_getfield(L, 3, "Y");
            lua_getfield(L, 3, "Z");
            
            float x = luaL_checknumber(L, -3);
            float y = luaL_checknumber(L, -2);
            float z = luaL_checknumber(L, -1);
            
            cube->pos = Vector3(x, y, z);
            cube->wakeUp();
            
            lua_pop(L, 3);
        }
        
        return 0;
    });
    lua_setfield(L, -2, "__newindex");
    
    lua_setglobal(L, "PartMetatable");
}

// Instance を Lua テーブルでラップ
void wrapInstance(lua_State* L, Instance* inst) {
    if (!inst) {
        lua_pushnil(L);
        return;
    }
    
    lua_newtable(L);
    
    // _ptr フィールドに lightuserdata を格納
    lua_pushlightuserdata(L, inst);
    lua_setfield(L, -2, "_ptr");
    
    // メタテーブルを設定
    if (inst->IsA("Part")) {
        lua_getglobal(L, "PartMetatable");
        lua_setmetatable(L, -2);
    }
}

// ===================================================================
// Workspace 登録
// ===================================================================

void registerWorkspace(lua_State* L) {
    lua_newtable(L);

    lua_pushcfunction(L, l_workspace_getPlayer);
    lua_setfield(L, -2, "getPlayer");
    
    lua_pushcfunction(L, l_workspace_FindFirstChild);
    lua_setfield(L, -2, "FindFirstChild");
    
    lua_pushcfunction(L, l_workspace_GetChildren);
    lua_setfield(L, -2, "GetChildren");

    lua_setglobal(L, "workspace");
}

// ===================================================================
// その他の関数
// ===================================================================

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

static int l_connect(lua_State* L) {
    EventBase* event = (EventBase*)lua_touserdata(L, lua_upvalueindex(1));

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

// ===================================================================
// 初期化
// ===================================================================

int initLua() {
    G_L = luaL_newstate();
    luaL_openlibs(G_L);

    // Part メタテーブル作成
    createPartMetatable(G_L);

    // RunService 登録
    lua_newtable(G_L);
    lua_newtable(G_L);
    lua_pushlightuserdata(G_L, &RunService::Heartbeat);
    lua_pushcclosure(G_L, l_connect, 1);
    lua_setfield(G_L, -2, "Connect");
    lua_setfield(G_L, -2, "Heartbeat");
    lua_setglobal(G_L, "RunService");

    // Workspace 登録
    registerWorkspace(G_L);

    // グローバル関数
    lua_register(G_L, "movePlayer", l_movePlayer);

    // Luaスクリプト実行
    if (luaL_dofile(G_L, "src/Game/script/hello.lua") != LUA_OK) {
        std::cerr << "Lua load error: " << lua_tostring(G_L, -1) << std::endl;
        lua_pop(G_L, 1);
    }

    return 0;
}