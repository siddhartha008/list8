// #include "generator.h"
// #include <stdexcept>
// #include <algorithm>

// void Generator::setList(const vector<int> &elements)
// {
//     if (elements.size() != 5)
//     {
//         throw invalid_argument("The list must contain exactly 5 elements.");
//     }

//     set<int> uniqueElements(elements.begin(), elements.end());
//     if (uniqueElements.size() != 5)
//     {
//         throw invalid_argument("List contains duplicate elements.");
//     }

//     listsGenerated.assign(uniqueElements.begin(), uniqueElements.end());
// }

// const std::vector<int> &Generator::getList() const
// {
//     return listsGenerated;
// }

// void Generator::addElements(const vector<int> &elements)
// {
//     for (int element : elements)
//     {
//         if (find(listsGenerated.begin(), listsGenerated.end(), element) != listsGenerated.end())
//         {
//             throw invalid_argument("Duplicate elements cannot be added.");
//         }
//         listsGenerated.push_back(element);
//     }
// }
