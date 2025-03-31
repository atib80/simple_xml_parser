#include "../include/simple_xml.h"
#include "stl_helper_functions.hpp"

#include <array>
#include <format>
#include <fstream>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace simple_xml
{

std::ostream &
operator<< (std::ostream &os, const XML &xml)
{
  xml.print (os);
  return os;
}

std::istream &
operator>> (std::istream &is, XML &xml)
{
  std::ostringstream oss;
  bool is_found_first_tag{}, is_parsed_header_information{};
  size_t pos{};
  xml_header_information xml_header_info;
  for (std::string line, header, end_tag_needle; std::getline (is, line);)
    {
      if (!is_parsed_header_information)
        {
          header.append (line);
          if (XML::parse_xml_header_information (header, pos,
                                                 &xml_header_info))
            {
              xml.header_information_ = std::move (xml_header_info);
              is_parsed_header_information = true;
              if (pos < header.length ())
                {
                  line = header.substr (pos);
                }
              else
                {
                  continue;
                }
            }
        }
      if (!is_found_first_tag)
        {
          if (size_t start_pos{}, end_pos{ std::string::npos };
              (start_pos = line.find ('<')) != std::string::npos
              && (end_pos = line.find ('>', start_pos + 1))
                     != std::string::npos
              && start_pos < end_pos)
            {
              end_pos = line.find_first_of ("> \t\n", start_pos + 1);
              end_tag_needle = std::format (
                  "</{}>", line.substr (start_pos + 1, end_pos));
              is_found_first_tag = true;
            }
        }
      oss << line;
      if (is_found_first_tag
          && line.find (end_tag_needle) != std::string::npos)
        break;
    }

  xml = XML::parse (oss.str ());
  return is;
}

void
skip_whitespace_characters (std::string_view str, size_t &pos)
{
  while (pos < str.size () && is_whitespace (str[pos]))
    ++pos;
}

bool
is_whitespace (const char ch)
{
  static constexpr std::array<bool, 1 << (sizeof (char) * 8u)> whitespaces
      = [] () consteval {
          std::array<bool, 1 << (sizeof (char) * 8u)> chars{};
          chars[static_cast<unsigned> (' ')] = true;
          chars[static_cast<unsigned> ('\t')] = true;
          chars[static_cast<unsigned> ('\n')] = true;
          chars[static_cast<unsigned> ('\r')] = true;
          chars[static_cast<unsigned> ('\f')] = true;
          chars[static_cast<unsigned> ('\v')] = true;
          return chars;
        }();
  return whitespaces[static_cast<unsigned char> (ch)];
}

XML
XML::parse (std::string_view input)
{
  size_t pos{ 0u };
  xml_header_information xml_header_info;
  parse_xml_header_information (input, pos, &xml_header_info);
  XML root{ parse_element (input, pos) };
  root.header_information_ = std::move (xml_header_info);
  return root;
}

XML
XML::parse (std::ifstream &input)
{
  if (!input || !input.is_open ())
    {
      throw std::invalid_argument{
        "Input file is in bad state or it is not opened!"
      };
    }
  XML root;
  input >> root;
  return root;
}

bool
XML::parse_xml_header_information (std::string_view str, size_t &pos,
                                   xml_header_information *xml_header_info)
{
  static constexpr std::string_view xml_header_info_start_tag{ "<?xml" };
  static constexpr std::string_view xml_header_info_end_tag{ "?>" };

  skip_whitespace_characters (str, pos);
  if (str.find (xml_header_info_start_tag, pos) == pos)
    {
      pos += xml_header_info_start_tag.length ();

      skip_whitespace_characters (str, pos);

      const size_t end_tag_pos{ str.find (xml_header_info_end_tag, pos) };
      if (std::string::npos == end_tag_pos)
        return false;

      std::string_view xml_header_tag_attributes (str.cbegin () + pos,
                                                  str.cbegin () + end_tag_pos);
      pos = end_tag_pos + xml_header_info_end_tag.length ();
      skip_whitespace_characters (str, pos);

      if (!xml_header_info)
        return true;

      auto attributes = str_split (xml_header_tag_attributes, " |\t|\n", "|",
                                   stl::helper::split_on_whole_needle_t::no,
                                   stl::helper::ignore_empty_string_t::yes);

      for (auto &attribute : attributes)
        {
          stl::helper::trim_in_place (attribute);
          auto attribute_pair
              = str_split (attribute, "=", nullptr,
                           stl::helper::split_on_whole_needle_t::yes);

          if (attribute_pair.size () != 2u)
            throw std::invalid_argument{
              "Invalid XML attribute syntax: a valid XML attribute must "
              "consist of 2 parts (a name and a value"
            };
          stl::helper::trim_in_place (attribute_pair[0]);
          stl::helper::trim_in_place (attribute_pair[1]);
          if (xml_header_info->header_attributes.contains (attribute_pair[0]))
            throw std::invalid_argument{ std::format (
                "Invalid XML attribute syntax: XML header attribute named {} "
                "has already been parsed once.",
                attribute_pair[0]) };
          xml_header_info->header_attributes.emplace (
              std::move (attribute_pair[0]), std::move (attribute_pair[1]));
        }

      return true;
    }

  return false;
}

XML
XML::parse_element (std::string_view str, size_t &pos)
{
  skip_whitespace_characters (str, pos);
  if (str[pos] != '<')
    throw std::invalid_argument{ std::format ("Expected '<' at position {}!",
                                              pos) };
  ++pos;

  std::string tag_name = parse_tag_name (str, pos);
  XML node (tag_name);

  skip_whitespace_characters (str, pos);
  while (str[pos] != '>' && str[pos] != '/')
    {
      auto [attribute_name, attribute_value] = parse_attribute (str, pos);
      node.attributes_.emplace (std::move (attribute_name),
                                std::move (attribute_value));
      skip_whitespace_characters (str, pos);
    }

  if (str[pos] == '/')
    {
      if (str[pos + 1] != '>')
        throw std::invalid_argument{ std::format (
            "Malformed self-closing tag at position {}!", pos) };
      pos += 2;
      return node;
    }

  ++pos;

  auto closing_tag_needle{ std::format ("</{}>", tag_name) };

  while (pos < str.length ()
         && str.substr (pos, closing_tag_needle.length ())
                != closing_tag_needle)
    {
      skip_whitespace_characters (str, pos);
      if (str[pos] == '<' && str[pos + 1] == '/')
        break;

      if (str[pos] == '<')
        {
          XML child = parse_element (str, pos);
          child.parent = &node;
          auto &node_children_for_child_tag_name
              = node.children_[child.tag_name_];
          node_children_for_child_tag_name.emplace_back (std::move (child));
        }
      else
        {
          node.tag_value_ += parse_text (str, pos);
        }
    }

  if (pos >= str.length ()
      || str.substr (pos, closing_tag_needle.length ()) != closing_tag_needle)
    {
      throw std::invalid_argument{ std::format (
          "Expected closing tag </{}> at position {}!", tag_name, pos) };
    }
  pos += closing_tag_needle.length ();

  return node;
}

std::string
XML::parse_tag_name (std::string_view str, size_t &pos)
{
  const size_t start{ pos };
  while (pos < str.length () && (std::isalnum (str[pos]) || str[pos] == '_'))
    ++pos;
  if (start == pos)
    throw std::invalid_argument{ std::format (
        "Invalid tag name at position {}!", pos) };
  return std::string (cbegin (str) + start, cbegin (str) + pos);
}

std::pair<std::string, std::string>
XML::parse_attribute (std::string_view str, size_t &pos)
{
  std::string attribute_name{ parse_tag_name (str, pos) };

  skip_whitespace_characters (str, pos);

  if (str[pos] != '=')
    throw std::invalid_argument{ std::format (
        "Expected '=' in attribute at position {}!", pos) };
  ++pos;
  skip_whitespace_characters (str, pos);

  if (str[pos] != '"')
    throw std::invalid_argument{ std::format (
        "Expected '\"' to start attribute value at position {}!", pos) };
  ++pos;
  const size_t start{ pos };
  while (pos < str.length () && str[pos] != '"')
    ++pos;
  if (pos >= str.length ())
    throw std::invalid_argument{ std::format (
        "Unterminated attribute value at position {}!", pos) };

  std::string attribute_value (cbegin (str) + start, cbegin (str) + pos);
  ++pos;

  return std::pair<std::string, std::string>{ attribute_name,
                                              attribute_value };
}

std::string
XML::parse_text (std::string_view str, size_t &pos)
{
  const size_t start{ pos };
  while (pos < str.length () && str[pos] != '<')
    ++pos;
  return std::string (cbegin (str) + start, cbegin (str) + pos);
}

std::vector<XML *>
XML::find_all_nodes_with_tag_name (std::string_view tag_name)
{
  std::vector<XML *> all_nodes_with_tag_name;
  auto &root_node = get_root_node ();
  if (root_node.tag_name_ == tag_name)
    all_nodes_with_tag_name.emplace_back (&root_node);

  std::stack<XML *> xml_nodes{ { &root_node } };

  while (!xml_nodes.empty ())
    {

      XML *current_node = xml_nodes.top ();
      xml_nodes.pop ();

      for (auto &[name, children] : current_node->children_)
        {
          for (auto &node : children)
            {
              if (name == tag_name)
                all_nodes_with_tag_name.emplace_back (&node);

              xml_nodes.emplace (&node);
            }
        }
    }

  return all_nodes_with_tag_name;
}

std::vector<const XML *>
XML::find_all_nodes_with_tag_name (std::string_view tag_name) const
{
  std::vector<const XML *> all_nodes_with_tag_name;
  const auto &root_node = get_root_node ();
  if (root_node.tag_name_ == tag_name)
    all_nodes_with_tag_name.emplace_back (&root_node);

  std::stack<const XML *> xml_nodes{ { &root_node } };

  while (!xml_nodes.empty ())
    {

      const XML *current_node = xml_nodes.top ();
      xml_nodes.pop ();

      for (const auto &[name, children] : current_node->children_)
        {
          for (const auto &node : children)
            {
              if (name == tag_name)
                all_nodes_with_tag_name.emplace_back (&node);

              xml_nodes.emplace (&node);
            }
        }
    }

  return all_nodes_with_tag_name;
}

XML &
XML::get_root_node ()
{
  XML *current{ this };
  while (current->parent)
    {
      current = current->parent;
    }

  return *current;
}

const XML &
XML::get_root_node () const
{
  const XML *current{ this };
  while (current->parent)
    {
      current = current->parent;
    }

  return *current;
}

void
XML::print (std::ostream &os, size_t indent) const
{
  os << std::format ("{}<{}", std::string (indent, ' '), tag_name_);
  for (const auto &[key, value] : attributes_)
    {
      os << std::format (R"( {}="{}")", key, value);
    }
  os << ">";

  if (!children_.empty ())
    {
      os << "\n";
      for (auto &&[key, nodes] : children_)
        {
          for (auto &&node : nodes)
            {
              node.print (os, indent + 2);
            }
        }
      os << std::string (indent, ' ');
    }
  else if (!tag_value_.empty ())
    {
      os << tag_value_;
    }

  os << std::format ("</{}>\n", tag_name_);
}

std::string
XML::to_json (size_t indent) const
{
  std::ostringstream oss;
  const XML &root_node = get_root_node ();
  oss << "{\n";
  root_node.to_json_helper (oss, indent, 0);
  oss << "\n}";
  return oss.str ();
}

void
XML::to_json_helper (std::ostringstream &oss, size_t indent,
                     size_t level) const
{
  const std::string padding (level * indent, ' ');
  oss << std::format (R"({}"{}": )", padding, tag_name_);

  if (!children_.empty () || !attributes_.empty ())
    {
      oss << "{\n";
      const std::string inner_padding ((level + 1) * indent, ' ');

      for (const auto &[key, value] : attributes_)
        {
          oss << std::format ("{}\"@{}\": \"{}\",\n", inner_padding, key,
                              value);
        }

      for (const auto &[key, nodes] : children_)
        {
          oss << inner_padding << "\"" << key << "\": [\n";
          // oss << inner_padding << ": [\n";
          for (const auto &node : nodes)
            {
              node.to_json_helper (oss, indent, level + 2);
              oss << ",\n";
            }
          oss << inner_padding << "],\n";
        }

      oss << padding << "}";
    }
  else
    {
      oss << "\"" << tag_value_ << "\"";
    }
}

}
