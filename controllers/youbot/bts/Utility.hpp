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

    bool compare(Node*& a, Node*& b)
    {
        return a -> query() > b -> query();
    }

    class Fallback : public Node
    {
        public:
        
        Fallback(std::vector<Node*> children = {})
        {
            setChildren(children);
        }

        Status tick()
        {
            std::sort(children.begin() + (children.front() -> getChildren().empty()), children.end(), compare); // pa-bt compatible

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
    };

    class Parallel : public Node
    {
        public:

        Parallel(std::vector<Node*> children = {}, int threshold = -1)
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
            if (threshold == -1)
                this -> threshold = children.size();
            else if (threshold >= 0 && threshold <= children.size())
                this -> threshold = threshold;

            return (threshold >= 1 && threshold <= children.size());
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
            reset();
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

            reset();
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
            reset();

            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        void reset()
        {
            index = 0;
        }

        private:
        int index;
    };

    class FallbackStar : public Fallback
    {
        public:

        FallbackStar(std::vector<Node*> children = {}) : Fallback(children)
        {
            reset();
        }

        Status tick()
        {
            if (!index) // only reorder when completed
                std::sort(children.begin() + (children.front() -> getChildren().empty()), children.end(), compare); // pa-bt compatible

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

            reset();
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
            reset();

            utility = 0;
            
            for (Node* child : children)
                utility = std::max(utility, child -> query());
        }

        void reset()
        {
            index = 0;
        }

        private:
        int index;
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

    class PercentSuccess : public Decorator
    {
        public:

        PercentSuccess(Node* child, int percent) : Decorator(child)
        {
            this -> percent = percent;
        }

        Status tick()
        {
            switch (child -> tick())
            {
                case FAILURE:
                    return FAILURE;

                case SUCCESS:
                    if (rand() % 100 < percent)
                        return SUCCESS;

                    return FAILURE;

                case RUNNING:
                    return RUNNING;
            }
        }

        private:
        int percent;
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
};