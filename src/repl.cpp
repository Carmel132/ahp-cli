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

auto promptUserForInt(const std::string &message) -> int
{

    std::println("{}", message);
    std::print("?> ");
    std::string response;
    std::getline(std::cin, response);
    std::optional<int> numReponse = tryParseInt(response);
    while (!numReponse.has_value())
    {
        std::print("Invalid input!\n?> ");
        std::getline(std::cin, response);
        numReponse = tryParseInt(response);
    }

    return numReponse.value();
}

auto promptUserForBool(const std::string &message) -> bool
{
    static std::vector<std::string> messages{"Yes", "No"};

    std::println("{}", message);
    return promptUserWithOption(messages) == 0;
}

auto promptUserForString(const std::string &message) -> std::string
{
    std::println("{}", message);
    std::print("?> ");
    std::string response;
    std::getline(std::cin, response);
    return response;
}

auto promptUserForHeadNode() -> std::pair<Node, std::vector<std::string>>
{
    int numLabels = promptUserForInt("How many labels are there");
    std::vector<std::string> labels(numLabels);

    for (int count{}; count < numLabels; ++count)
    {
        labels[count] = promptUserForString(std::format("Label {}", count + 1));
    }

    std::string objective = promptUserForString("What is the objective");

    return std::make_pair(Node{objective, SquareMatrix::OneMatrix(numLabels), std::nullopt, std::nullopt}, labels);
}

auto Node::collectSubcriteriaNames()
{
    assert(subcriteria_.has_value() && "Cannot collect subcriteria names without subcriteria!");

    return subcriteria_.value() | std::views::transform(&Node::name_);
}

auto promptUserForNode(Node &currentNode, const std::vector<std::string> &labels) -> Node
{
    static std::array<NodePromptOption, 4> options{{
        {[](Node &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change name";
         },
         [](Node &node, const std::vector<std::string> &strings) -> Node
         {
             node.name_ = promptUserForString("What will be the new name?");
             return node;
         },
         [](Node &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {[](Node &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change matrix";
         },
         [](Node &node, const std::vector<std::string> &strings) -> Node
         {
             if (node.subcriteria_.has_value())
             {
                 node.matrix_ = promptUserForWeightMatrix(node.collectSubcriteriaNames());
             }
             else
             {
                 node.matrix_ = promptUserForWeightMatrix(strings);
             }
             return node;
         },
         [](Node &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {[](Node &node, const std::vector<std::string> &strings) -> std::string
         {
             return node.subcriteria_.has_value() ? "See subcriteria" : "Add subcriteria";
         },
         [](Node &node, const std::vector<std::string> &strings) -> Node
         {
             if (node.subcriteria_.has_value())
             {
                 return node.subcriteria_.value().at(promptUserWithOption(node.collectSubcriteriaNames()));
             }

             int numSubcriteria = promptUserForInt("How many subcriteria are there");

             // TODO: Make vector field into a smart pointer so it doesn't delete on lambda exit
             std::vector<Node> subcriteriaNodes{};
             subcriteriaNodes.reserve(numSubcriteria);

             for (int count{1}; count <= numSubcriteria; ++count)
             {
                 std::string nodeName = promptUserForString(std::format("Subcriteria {}", count));
                 subcriteriaNodes.emplace_back(Node{nodeName, SquareMatrix::OneMatrix(strings.size()), std::nullopt, std::make_optional(std::make_shared<Node>(node))});
             }
             node.subcriteria_ = std::make_optional(std::move(subcriteriaNodes));
             return std::move(node);
         },
         [](Node &node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {[](Node &node, const std::vector<std::string> &strings) -> std::string
         {
             return "Go to enclosing criteria";
         },
         [](Node &node, const std::vector<std::string> &strings) -> Node
         {
             assert(node.enclosingCriteria_.has_value() && "Must have enclosing criteria to go up!");

             return *node.enclosingCriteria_.value();
         },
         [](Node &node, const std::vector<std::string> &strings) -> bool
         {
             return node.enclosingCriteria_.has_value();
         }},
    }};

    std::print("Current node name: {}", currentNode.name_);

    int opt = promptUserWithOption(options | std::views::transform([&currentNode, labels](NodePromptOption &option) -> std::string
                                                                   { return option.getName(currentNode, labels); }));

    currentNode = options.at(opt).promptAction(currentNode, labels);

    return currentNode;
}
