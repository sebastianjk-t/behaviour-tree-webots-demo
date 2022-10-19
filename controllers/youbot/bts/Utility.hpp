#pragma once
#include <vector>

namespace Utility
{
    enum Status{FAILURE, SUCCESS, RUNNING};

    class Node;
    Node* spare;

    class Node
    {
        public:

        virtual Status tick() = 0;

        virtual std::vector<Node*> getChildren()
        {
            return {};
        }

        virtual Node*& getFailure()
        {
            return spare;
        }

        virtual int query(bool update = false)
        {
            return utility;
        }

        void setUtility(int utility)
        {
            this -> utility = utility;
        }

        protected:
        int utility = 0;
    };

    class Sequence : public Node
    {
        public:

        Sequence(std::vector<Node*> children = {})
        {
            setChildren(children);
        }

        Status tick()
        {
            for (Node* child : children)
            {
                switch (child -> tick())
                {                
                    case RUNNING:
                        return RUNNING;

                    case FAILURE:
                        return FAILURE;

                    case SUCCESS:
                        break;
                }
            }

            return SUCCESS;
        }

        std::vector<Node*> getChildren()
        {
            return children;
        }

        Node*& getFailure()
        {
            for (Node*& child : children)
            {
                if (child -> tick() == FAILURE)
                {                
                    if (child -> getChildren().empty())
                        return child;

                    return child -> getFailure();
                }
            }

            return children.back() -> getFailure();
        }

        int query(bool update = false)
        {
            if (update)
            {
                utility = 0;

                for (Node* child : children)
                    utility = std::max(utility, child -> query(true));
            }

            return utility;
        }

        void setChildren(std::vector<Node*> children)
        {
            this -> children = children;
            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        protected:
        std::vector<Node*> children;
    };

    bool compare(Node* a, Node* b)
    {
        return a -> query() < b -> query();
    }

    class Fallback : public Node
    {
        public:
        
        Fallback(std::vector<Node*> children = {}, bool isUtility = true)
        {
            setChildren(children);
            this -> isUtility = isUtility;
        }

        Status tick()
        {
            if (isUtility)
                std::sort(children.begin(), children.end(), compare);

            for (Node* child : children)
            {
                switch (child -> tick())
                {                
                    case RUNNING:
                        return RUNNING;
                    
                    case SUCCESS:
                        return SUCCESS;

                    case FAILURE:
                        break;
                }
            }

            return FAILURE;
        }

        std::vector<Node*> getChildren()
        {
            return children;
        }

        Node*& getFailure()
        {
            if (children.back() -> getChildren().empty())
                return children.back();

            return children.back() -> getFailure();
        }

        int query(bool update = false)
        {
            if (update)
            {
                utility = 0;

                for (Node* child : children)
                    utility = std::max(utility, child -> query(true));
            }

            return utility;
        }

        void setChildren(std::vector<Node*> children)
        {
            this -> children = children;
            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        protected:
        std::vector<Node*> children;
        bool isUtility;
    };

    class Parallel : public Node
    {
        public:

        Parallel(std::vector<Node*> children = {}, int threshold = 0)
        {
            setChildren(children);
            setThreshold(threshold);
        }

        Status tick()
        {
            int successes = 0, failures = 0;

            for (Node* child : children)
            {
                switch (child -> tick())
                {                       
                    case SUCCESS:
                        successes++;
                        break;

                    case FAILURE:
                        failures++;
                        break;

                    case RUNNING:
                        break;
                }
            }

            if (successes >= threshold)
                return SUCCESS;

            if (failures > children.size() - threshold)
                return FAILURE;

            return RUNNING;
        }

        std::vector<Node*> getChildren()
        {
            return children;
        }

        Node*& getFailure()
        {
            for (Node*& child : children)
            {
                if (child -> tick() == FAILURE)
                {                
                    if (child -> getChildren().empty())
                        return child;

                    return child -> getFailure();
                }
            }

            return children.back() -> getFailure();
        }

        int query(bool update = false)
        {
            if (update)
            {
                utility = 0;

                for (Node* child : children)
                    utility = std::max(utility, child -> query(true));
            }

            return utility;
        }

        void setChildren(std::vector<Node*> children)
        {
            this -> children = children;
            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        bool setThreshold(int threshold)
        {
            if (threshold >= 0 && threshold <= children.size())
                this -> threshold = threshold;

            return (threshold >= 0 && threshold <= children.size());
        }

        protected:

        std::vector<Node*> children;
        int threshold;
    };

    class Decorator : public Node
    {
        public:

        Decorator(Node* child = nullptr)
        {
            setChild(child);
        }

        Status tick()
        {
            return child -> tick();
        }

        std::vector<Node*> getChildren()
        {
            return {child};
        }

        Node*& getFailure()
        {
            if (child -> getChildren().empty())
                return child;

            return child -> getFailure();
        }

        int query(bool update = false)
        {
            if (update)
                utility = std::max(utility, child -> query(true));

            return utility;
        }

        void setChild(Node* child)
        {
            this -> child = child;
            this -> utility = child -> query();
        }

        protected:
        Node* child;
    };

    class SequenceStar : public Sequence
    {
        public:

        SequenceStar(std::vector<Node*> children = {}) : Sequence(children)
        {
            
        }

        Status tick()
        {
            int n = children.size();

            while (index < n)
            {
                switch (children[index] -> tick())
                {                
                    case RUNNING:
                        return RUNNING;

                    case FAILURE:
                        return FAILURE;

                    case SUCCESS:
                        break;
                }

                index++;
            }

            index = 0;

            return SUCCESS;
        }

        Node*& getFailure()
        {
            if (children[index] -> getChildren().empty())
                return children[index];

            return children[index] -> getFailure();
        }

        void setChildren(std::vector<Node*> children)
        {
            this -> children = children;
            index = 0;

            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        private:
        int index = 0;
    };

    class FallbackStar : public Fallback
    {
        public:

        FallbackStar(std::vector<Node*> children = {}, bool isUtility = true) : Fallback(children, isUtility)
        {
            
        }

        Status tick()
        {
            if (isUtility && !index) // only reorder when completed
                std::sort(children.begin(), children.end(), compare);

            int n = children.size();

            while (index < n)
            {
                switch (children[index] -> tick())
                {                
                    case RUNNING:
                        return RUNNING;

                    case SUCCESS:
                        return SUCCESS;

                    case FAILURE:
                        break;
                }

                index++;
            }

            index = 0;

            return FAILURE;
        }

        Node*& getFailure()
        {
            if (children[index] -> getChildren().empty())
                return children[index];

            return children[index] -> getFailure();
        }

        void setChildren(std::vector<Node*> children)
        {
            this -> children = children;
            index = 0;

            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        private:
        int index = 0;
    };

    class Inverter : public Decorator
    {
        public:

        Inverter(Node* child) : Decorator(child)
        {
            
        }

        Status tick()
        {
            switch (child -> tick())
            {
                case FAILURE:
                    return SUCCESS;

                case SUCCESS:
                    return FAILURE;

                case RUNNING:
                    return RUNNING;
            }
        }
    };

    template <typename... Ts>
    class Action : public Node
    {
        public:

        Action(std::function<Status(Ts...)> action, Ts... args, int utility)
        {
            this -> action = action;
            setArgs(args...);
            setUtility(utility);
        }

        Status tick()
        {
            return std::apply(action, args);
        }

        void setArgs(Ts... args)
        {
            this -> args = {args...};
        }

        protected:
        std::function<Status(Ts...)> action;
        std::tuple<Ts...> args;
    };

    template <typename... Ts>
    class Condition : public Node
    {
        public:
        Condition(std::function<bool(Ts...)> condition, Ts... args, int utility)
        {
            this -> condition = condition;
            setArgs(args...);
            setUtility(utility);
        }

        Status tick()
        {
            if (std::apply(condition, args))
                return SUCCESS;
            
            return FAILURE;
        }

        void setArgs(Ts... args)
        {
            this -> args = {args...};
        }

        protected:
        std::function<bool(Ts...)> condition;
        std::tuple<Ts...> args;
    };

    template <typename T, typename... Ts>
    class ForcedAction : public Node
    {
        public:

        ForcedAction(std::function<T(Ts...)> action, Ts... args, int utility)
        {
            this -> action = action;
            setArgs(args...);
            setUtility(utility);
        }

        Status tick()
        {
            std::apply(action, args);
            return SUCCESS;
        }

        void setArgs(Ts... args)
        {
            this -> args = {args...};
        }

        protected:
        std::function<T(Ts...)> action;
        std::tuple<Ts...> args;
    };
};