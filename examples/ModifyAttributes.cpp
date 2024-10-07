#include <HtmlParser/Parser.hpp>
#include <iostream>

int main()
{
    // HTML with an image element
    std::string Html = "<img src=\"old_image.jpg\" alt=\"Old Image\">";

    HtmlParser::Parser Parser;
    HtmlParser::DOM DOM = Parser.Parse(Html);

    // Get the <img> element
    auto Image = DOM.GetElementsByTagName("img").front();

    if (Image)
    {
        // Output the original src attribute
        std::cout << "Original src: " << Image->GetAttribute("src") << "\n";

        // Modify the src attribute
        Image->SetAttribute("src", "new_image.jpg");

        // Output the modified src attribute
        std::cout << "Modified src: " << Image->GetAttribute("src") << "\n";
    }
    else
    {
        std::cout << "Image element not found.\n";
    }

    return 0;
}