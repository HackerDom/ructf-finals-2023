#include "parser.h"


NodesList DcsProgramNode::GetChildNodes() const {
    NodesList result;

    result.reserve(Constants.size() + Functions.size());

    for (const auto &p : Constants) {
        result.push_back(p);
    }

    for (const auto &p : Functions) {
        result.push_back(p);
    }

    return result;
}
