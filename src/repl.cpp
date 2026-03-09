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
    static std::array<NodePromptOption, 4> options{{
        {.getName = [](Node_ptr& node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change name";
         },
         .promptAction = [&input](Node_ptr& node, const std::vector<std::string> &strings) -> Node_ptr
         {
             node->name_ = promptUserForString("What will be the new name?", input);
             return node;
         },
         .isAvailable = [](Node_ptr& node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr& node, const std::vector<std::string> &strings) -> std::string
         {
             return "Change matrix";
         },
         .promptAction = [&input](Node_ptr& node, const std::vector<std::string> &strings) -> Node_ptr
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
         .isAvailable = [](Node_ptr& node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr& node, const std::vector<std::string> &strings) -> std::string
         {
             return node->subcriteria_.has_value() ? "See subcriteria" : "Add subcriteria";
         },
         .promptAction = [&input](Node_ptr& node, const std::vector<std::string> &strings) -> Node_ptr
         {
             if (node->subcriteria_.has_value())
             {
                 return node->subcriteria_.value().at(promptUserWithOption(node->collectSubcriteriaNames()));
             }

             int numSubcriteria = promptUserForInt("How many subcriteria are there");

             // TODO: Make vector field into a smart pointer so it doesn't delete on lambda exit
             std::vector<Node_ptr> subcriteriaNodes{};
             subcriteriaNodes.reserve(numSubcriteria);

             for (int count{1}; count <= numSubcriteria; ++count)
             {
                 std::string nodeName = promptUserForString(std::format("Subcriteria {}", count), input);
              
                auto child = std::make_shared<Node>(
                    nodeName,
                    SquareMatrix::OneMatrix(strings.size())
                );

                child->enclosingCriteria_ = node;
                subcriteriaNodes.emplace_back(child);    
            }

             node->subcriteria_.emplace(subcriteriaNodes);
             return node;
         },
         .isAvailable = [](Node_ptr& node, const std::vector<std::string> &strings) -> bool
         {
             return true;
         }},
        {.getName = [](Node_ptr& node, const std::vector<std::string> &strings) -> std::string
         {
             return "Go to enclosing criteria";
         },
         .promptAction = [](Node_ptr& node, const std::vector<std::string> &strings) -> Node_ptr
         {
             assert(node->enclosingCriteria_.has_value() && "Must have enclosing criteria to go up!");

             auto enclosing = node->enclosingCriteria_.value().lock();

             return enclosing;
         },
         .isAvailable = [](Node_ptr& node, const std::vector<std::string> &strings) -> bool
         {
             return node->enclosingCriteria_.has_value();
         }},
    }};

    std::println("Current node name: {}", currentNode->name_);

    std::vector<size_t> validOptionIndices{};
    validOptionIndices.reserve(options.size());
    for (size_t idx{}; idx < options.size(); ++idx) {
        if (options.at(idx).isAvailable(currentNode, labels)) {
            validOptionIndices.emplace_back(idx);
        }
    }

    auto displayStrings = validOptionIndices
    | std::views::transform([&](size_t idx) -> std::string {return options.at(idx).getName(currentNode, labels);});

    size_t opt = validOptionIndices.at(promptUserWithOption(displayStrings, input));

    currentNode = options.at(opt).promptAction(currentNode, labels);

    return currentNode;
}
