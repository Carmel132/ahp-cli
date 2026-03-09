#include <repl.h>

auto tryParseInt(std::string_view str) -> std::optional<int>
{
    int value;
    auto [ptr, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        value);

    if (ec == std::errc() && ptr == str.data() + str.size())
    {
        return value;
    }

    return std::nullopt;
}

auto promptUserForInt(const std::string &message, std::istream &input) -> int
{

    std::println("{}", message);
    std::print("?> ");
    std::string response;
    std::getline(input, response);
    std::optional<int> numReponse = tryParseInt(response);
    while (!numReponse.has_value())
    {
        std::print("Invalid input!\n?> ");
        std::getline(input, response);
        numReponse = tryParseInt(response);
    }

    return numReponse.value();
}

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

auto promptUserForHeadNode(std::istream &input) -> std::pair<Node, std::vector<std::string>>
{
    int numLabels = promptUserForInt("How many labels are there", input);
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
    static std::array<NodePromptOption, 8> options{{
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
             //  if (node->subcriteria_.has_value())
             //  {
             //      return node->subcriteria_.value().at(promptUserWithOption(node->collectSubcriteriaNames()));
             //  }

             int numSubcriteria = promptUserForInt("How many subcriteria do you want to add");

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

             if (node->subcriteria_.value().size() == 0)
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

    }};

    std::println("Current node name: {}", currentNode->name_);

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
