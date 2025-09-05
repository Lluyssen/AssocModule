#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>
#include <functional>
#include <string>

struct Network 
{
    template <typename T> void send(const T& obj)       { std::cout << "Send: "   << toString(obj) << std::endl; }
    template <typename T> void sendCreate(const T& obj) { std::cout << "Create: " << toString(obj) << std::endl; }
    template <typename T> void sendUpdate(const T& obj) { std::cout << "Update: " << toString(obj) << std::endl; }
};

class AssocModule
{
    private:

        Network& network;

        struct IPool { virtual ~IPool(void) = default; };

        template <typename T>
        struct Pool : IPool 
        {
            using Comparator = bool(*)(const T&, const T&);
            Comparator comparator = nullptr;
            std::vector<T> storage;

            Pool(Comparator cmp) : comparator(cmp) {}

            void process(Network& net, const T& obj)
            {
                if (!comparator)
                { 
                    net.send(obj); 
                    return; 
                }

                for (auto& existing : storage)
                {
                    if (comparator(existing, obj))
                    {
                        net.sendUpdate(obj);
                        return;
                    }
                }
                storage.push_back(obj);
                net.sendCreate(obj);
            }

            void erase(const T& obj)
            {
                if (!comparator)
                    return;
                storage.erase(std::remove_if(storage.begin(), storage.end(), [&](const T& existing) { 
                    return comparator(existing, obj); }), storage.end());
            }
        };

        std::unordered_map<std::type_index, std::unique_ptr<IPool>> pools;

    public:

        explicit AssocModule(Network& net) : network(net) {}
        virtual ~AssocModule(void) = default;

        template <typename T>
        void registerType(bool(*cmp)(const T&, const T&))
        {
            pools[std::type_index(typeid(T))] = std::make_unique<Pool<T>>(cmp);
        }

        template <typename T>
        void process(const T& obj)
        {
            auto it = pools.find(std::type_index(typeid(T)));
            if (it != pools.end())
            {
                auto* pool = static_cast<Pool<T>*>(it->second.get());
                pool->process(network, obj);
            } 
            else
                network.send(obj);
        }

        template <typename T>
        void erase(const T& obj)
        {
            auto it = pools.find(std::type_index(typeid(T)));
            if (it != pools.end())
            {
                auto* pool = static_cast<Pool<T>*>(it->second.get());
                pool->erase(obj);
            }
        }

        template <typename T>
        void erasePull(void)
        {
            pools.erase(std::type_index(typeid(T)));
        }

        void eraseAll(void)
        {
            pools.clear();
        }
};

struct Player { int id; std::string name; };
struct Position { int code; double value; };
struct Teleportation { float x, y; };

std::string toString(const Player& d) { return "Player {" + std::to_string(d.id) + "," + d.name + "}"; }
std::string toString(const Position& d) { return "Position {" + std::to_string(d.code) + "," + std::to_string(d.value) + "}"; }
std::string toString(const Teleportation& d) { return "Teleportation {" + std::to_string(d.x) + "," + std::to_string(d.y) + "}"; }

int main(void)
{
    Network net;
    AssocModule module(net);

    module.registerType<Player>([](const Player& a, const Player& b){ return a.id == b.id; });
    module.registerType<Position>([](const Position& a, const Position& b){ return a.code == b.code; });

    module.process(Player{1,"Victor"});       // Create
    module.process(Player{1,"Toto"});    // Update
    module.process(Position{42,3.14});         // Create
    module.process(Position{42,2.71});         // Update
    module.process(Player{2,"EVA"});         // Create
    module.process(Teleportation{10.0f,20.0f});     // pas enregistré -> Send

    module.erase(Player{1,"mort"});        // supprime Alice
    return 0;
}
