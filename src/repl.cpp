#include <repl.h>

auto promptUserForBool(const std::string &message, std::istream &input) -> bool
{
    static std::vector<std::string> messages{"Yes", "No"};

    std::println("{}", message);
    return promptUserWithOption(messages, input) == 0;
}

auto promptUserForString(const std::string &message, std::istream &input) -> std::string
{
    std::println("{}", message);
    std::print("?> ");
    std::string response;
    std::getline(input, response);
    return response;
}

void printMatrix(Node_ptr &node)
{

    for (size_t row = 0; row < node->matrix_.size(); ++row)
    {
        std::print("[");
        for (size_t column = 0; column < node->matrix_.size(); ++column)
        {
            std::print("{:>5.3g}", node->matrix_.get(row, column));
            if (column < node->matrix_.size() - 1)
            {
                std::print("  ");
            }
        }
        std::println("]");
    }
}

void invalidateMatrix(Node_ptr &node)
{
    node->matrix_ = SquareMatrix::OneMatrix(node->subcriteria_.has_value() ? node->subcriteria_.value().size() : 0);
}

auto getRootNode(const Node_ptr &node) -> Node_ptr
{
    Node_ptr currentNode = node;
    while (currentNode->enclosingCriteria_.has_value())
    {
        currentNode = node->enclosingCriteria_.value().lock();
    }

    return currentNode;
}

auto addYAMLSubcriteria(YAML::Node &head, const Node_ptr &node) -> YAML::Node
{
    head["name"] = node->name_;

    head["comparisons"] = node->matrix_.getUnderlying();

    if (node->subcriteria_.has_value())
    {
        for (Node_ptr &subcriteria : node->subcriteria_.value())
        {

            YAML::Node subcriteriaNode;
            head["subcriteria"].push_back(addYAMLSubcriteria(subcriteriaNode, subcriteria));
        }
    }

    return head;
}

auto convertToYAMLNode(const Node_ptr &node, const std::vector<std::string> &labels) -> YAML::Node
{
    YAML::Node ret;

    ret["labels"] = labels;

    YAML::Node criteria;

    addYAMLSubcriteria(criteria, node);

    ret["criteria"] = criteria;

    return ret;
}

auto fromYAMLNodeSubcriteria(Node_ptr& enclosing, const YAML::Node& subcriteria) -> Node_ptr {
    auto name = subcriteria["name"].as<std::string>();
    auto mat = SquareMatrix(subcriteria["comparisons"].as<std::vector<std::vector<double>>>());

    std::optional<std::vector<Node_ptr>> sub = std::nullopt;

    Node_ptr ret = std::make_shared<Node>(name, mat, sub, enclosing);

    if (YAML_MapHasLabel(subcriteria, "subcriteria")) {
        ret->subcriteria_.emplace();
        for (YAML::Node subcriteriaNode : subcriteria["subcriteria"]) {
            ret->subcriteria_.value().emplace_back(fromYAMLNodeSubcriteria(ret, subcriteriaNode));
        }
    }

    return ret;
}

auto fromYAMLNode(const YAML::Node &head) -> std::pair<Node_ptr, std::vector<std::string>>
{
    auto labels = head["labels"].as<std::vector<std::string>>();
    auto objective = head["criteria"]["name"].as<std::string>();
    auto mat = SquareMatrix(head["criteria"]["comparisons"].as<std::vector<std::vector<double>>>());

    Node_ptr ret = std::make_shared<Node>(objective, mat, std::nullopt, std::nullopt);

    if (YAML_MapHasLabel(head["criteria"], "subcriteria")) {
        ret->subcriteria_.emplace();
        for (YAML::Node subcriteriaNode : head["criteria"]["subcriteria"]) {
            ret->subcriteria_.value().emplace_back(fromYAMLNodeSubcriteria(ret, subcriteriaNode));
        }
    }

    return std::make_pair(ret, labels);
}

auto promptUserForHeadNode(std::istream &input) -> std::pair<Node, std::vector<std::string>>
{
    int numLabels = promptUserForNumeric<int>("How many labels are there", input);
    std::vector<std::string> labels(numLabels);

    for (int count{}; count < numLabels; ++count)
    {
        labels[count] = promptUserForString(std::format("Label {}", count + 1), input);
    }

    std::string objective = promptUserForString("What is the objective", input);

    return std::make_pair(Node(objective, SquareMatrix::OneMatrix(numLabels), std::nullopt, std::nullopt), labels);
}

auto Node::collectSubcriteriaNames()
{
    assert(subcriteria_.has_value() && "Cannot collect subcriteria names without subcriteria!");

    return (subcriteria_.value()) | std::views::transform(&Node::name_);
}

auto promptUserForNode(Node_ptr currentNode, const std::vector<std::string> &labels, std::istream &input) -> Node_ptr
{
    static const int numOptions = 8;

    static std::array<NodePromptOption, numOptions> options{{
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change name";
         },
         .promptAction = [&input](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             node->name_ = promptUserForString("What will be the new name?", input);
             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change matrix";
         },
         .promptAction = [&input](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             if (node->subcriteria_.has_value())
             {
                 node->matrix_ = promptUserForWeightMatrix(node->collectSubcriteriaNames(), input);
             }
             else
             {
                 node->matrix_ = promptUserForWeightMatrix(strings, input);
             }
             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Add subcriteria";
         },
         .promptAction = [&input](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             int numSubcriteria = promptUserForNumeric<int>("How many subcriteria do you want to add", input);

             std::vector<Node_ptr> subcriteriaNodes{};
             subcriteriaNodes.reserve(numSubcriteria);

             for (int count{1}; count <= numSubcriteria; ++count)
             {
                 std::string nodeName = promptUserForString(std::format("Subcriteria {}", count), input);

                 auto child = std::make_shared<Node>(
                     nodeName,
                     SquareMatrix::OneMatrix(strings.size()));

                 child->enclosingCriteria_ = node;
                 subcriteriaNodes.emplace_back(child);
             }

             if (node->subcriteria_.has_value())
             {
                 node->subcriteria_.value().append_range(subcriteriaNodes);
             }
             else
             {
                 node->subcriteria_.emplace(subcriteriaNodes);
             }

             invalidateMatrix(node);
             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Remove subcriteria";
         },
         .promptAction = [&input](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             std::println("{}", "Select a subcriteria to remove");

             int option = promptUserWithOption(node->collectSubcriteriaNames(), input);

             node->subcriteria_.value().erase(node->subcriteria_.value().begin() + option);

             if (node->subcriteria_.value().empty())
             {
                 node->subcriteria_.reset(); // Ensure subcriteria vector is not empty
             }

             invalidateMatrix(node);
             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return node->subcriteria_.has_value();
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "See subcriteria";
         },
         .promptAction = [&input](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             std::println("{}", "Select a subcriteria");

             return node->subcriteria_.value().at(promptUserWithOption(node->collectSubcriteriaNames()));
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return node->subcriteria_.has_value();
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Go to enclosing criteria";
         },
         .promptAction = [](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             assert(node->enclosingCriteria_.has_value() && "Must have enclosing criteria to go up!");

             auto enclosing = node->enclosingCriteria_.value().lock();

             return enclosing;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return node->enclosingCriteria_.has_value();
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Export";
         },
         .promptAction = [&](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             std::ofstream fout(promptUserForString("File name", input));

             fout << convertToYAMLNode(getRootNode(node), strings);
             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Run";
         },
         .promptAction = [&](Node_ptr &node, const std::vector<std::string> &strings) -> Node_ptr
         {
             std::println("\n====================\nDecision: {}\n====================\n", getDecision(convertToYAMLNode(getRootNode(node), strings)));

             return node;
         },
         .isAvailable = [](Node_ptr &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
    }};

    std::println("Current node name: {}", currentNode->name_);
    printMatrix(currentNode);

    double consistencyRaw = getConsistency(currentNode->matrix_, getWeightVector(currentNode->matrix_));
    std::println("Raw consistency: {:.3g}, Consistency index: {:.3g}", consistencyRaw, getConsistencyIndex(consistencyRaw, labels.size()));

    std::vector<size_t> validOptionIndices{};
    validOptionIndices.reserve(options.size());
    for (size_t idx{}; idx < options.size(); ++idx)
    {
        if (options.at(idx).isAvailable(currentNode, labels))
        {
            validOptionIndices.emplace_back(idx);
        }
    }

    auto displayStrings = validOptionIndices | std::views::transform([&](size_t idx) -> std::string
                                                                     { return options.at(idx).getName(currentNode, labels); });

    size_t opt = validOptionIndices.at(promptUserWithOption(displayStrings, input));

    currentNode = options.at(opt).promptAction(currentNode, labels);

    return currentNode;
}
