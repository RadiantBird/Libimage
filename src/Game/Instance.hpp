// src/Game/Instance.hpp
#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <string>
#include <vector>
#include <memory>

// 前方宣言
class Instance;

// Instanceの基底クラス（Robloxライク）
class Instance {
public:
    std::string Name;
    std::string ClassName;
    Instance* Parent;
    std::vector<Instance*> Children;

    Instance(const std::string& name = "Instance", const std::string& className = "Instance")
        : Name(name), ClassName(className), Parent(nullptr) {}

    virtual ~Instance() {
        // 子を削除
        for (auto* child : Children) {
            child->Parent = nullptr;
            delete child;
        }
        Children.clear();
    }

    // 子を追加
    void addChild(Instance* child) {
        if (!child) return;
        
        // 既に親がいる場合は削除
        if (child->Parent) {
            child->Parent->removeChild(child);
        }
        
        child->Parent = this;
        Children.push_back(child);
    }

    // 子を削除
    void removeChild(Instance* child) {
        auto it = std::find(Children.begin(), Children.end(), child);
        if (it != Children.end()) {
            (*it)->Parent = nullptr;
            Children.erase(it);
        }
    }

    // FindFirstChild (名前で検索) - virtual にする
    virtual Instance* FindFirstChild(const std::string& name, bool recursive = false) {
        // 直接の子を検索
        for (auto* child : Children) {
            if (child->Name == name) {
                return child;
            }
        }
        
        // 再帰的に検索
        if (recursive) {
            for (auto* child : Children) {
                Instance* result = child->FindFirstChild(name, true);
                if (result) return result;
            }
        }
        
        return nullptr;
    }

    // FindFirstChildOfClass (クラス名で検索)
    Instance* FindFirstChildOfClass(const std::string& className, bool recursive = false) {
        for (auto* child : Children) {
            if (child->ClassName == className) {
                return child;
            }
        }
        
        if (recursive) {
            for (auto* child : Children) {
                Instance* result = child->FindFirstChildOfClass(className, true);
                if (result) return result;
            }
        }
        
        return nullptr;
    }

    // GetChildren (全ての子を取得)
    std::vector<Instance*> GetChildren() {
        return Children;
    }

    // GetDescendants (全ての子孫を取得)
    std::vector<Instance*> GetDescendants() {
        std::vector<Instance*> descendants;
        for (auto* child : Children) {
            descendants.push_back(child);
            auto childDescendants = child->GetDescendants();
            descendants.insert(descendants.end(), childDescendants.begin(), childDescendants.end());
        }
        return descendants;
    }

    // IsA (型チェック)
    virtual bool IsA(const std::string& className) const {
        return ClassName == className || className == "Instance";
    }

    // 仮想関数（オーバーライド可能）
    virtual void onAdded() {}
    virtual void onRemoved() {}
};

#endif // INSTANCE_HPP