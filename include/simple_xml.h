#ifndef SIMPLE_XML_H
#define SIMPLE_XML_H

#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace simple_xml
{

void skip_whitespace_characters (std::string_view str, size_t &pos);

bool is_whitespace (const char ch);

class XML;

std::ostream &operator<< (std::ostream &os, const XML &xml);

std::istream &operator>> (std::istream &is, XML &xml);

struct xml_header_information
{
  std::unordered_map<std::string, std::string> header_attributes;
};

class XML
{
public:
  using xml_node = std::unordered_map<std::string, std::vector<XML>>;
  using xml_node_attributes = std::unordered_map<std::string, std::string>;

  XML () = default;
  XML (std::string_view tag_name, std::string_view tag_value = "",
       XML *parent = nullptr)
      : tag_name_ (tag_name), tag_value_ (tag_value), parent{ parent }
  {
  }

  friend std::ostream &operator<< (std::ostream &os, const XML &xml);
  friend std::istream &operator>> (std::istream &is, XML &xml);

  static XML parse (std::string_view input);
  static XML parse (std::ifstream &input);

  std::vector<XML *> find_all_nodes_with_tag_name (std::string_view tag_name);

  std::vector<const XML *>
  find_all_nodes_with_tag_name (std::string_view tag_name) const;

  XML &get_root_node ();

  const XML &get_root_node () const;

  XML *
  get_parent_node () noexcept
  {
    return parent;
  }

  const XML *
  get_parent_node () const noexcept
  {
    return parent;
  }

  std::unordered_map<std::string, std::vector<XML>> &
  get_children_nodes () noexcept
  {
    return children_;
  }

  const std::unordered_map<std::string, std::vector<XML>> &
  get_children_nodes () const noexcept
  {
    return children_;
  }

  void print (std::ostream &os, size_t indent = 0) const;

  std::string to_json (size_t indent = 0) const;

  explicit
  operator bool () const noexcept
  {
    return tag_name_.empty ();
  }

  bool
  is_root_node () const noexcept
  {
    return !parent;
  }

  bool
  is_empty_node () const noexcept
  {
    return children_.empty () && tag_value_.empty ();
  }

  bool
  is_leaf_node () const noexcept
  {
    return children_.empty ();
  }

  bool
  has_children () const noexcept
  {
    return !children_.empty ();
  }

private:
  std::string tag_name_;
  std::string tag_value_;
  xml_node children_;
  XML *parent{ nullptr };
  xml_node_attributes attributes_;
  xml_header_information header_information_;

  static bool
  parse_xml_header_information (std::string_view str, size_t &pos,
                                xml_header_information *xml_header_info
                                = nullptr);

  static XML parse_element (std::string_view str, size_t &pos);

  static std::string parse_tag_name (std::string_view str, size_t &pos);

  static std::pair<std::string, std::string>
  parse_attribute (std::string_view str, size_t &pos);

  static std::string parse_text (std::string_view str, size_t &pos);

  void to_json_helper (std::ostringstream &oss, size_t indent,
                       size_t level) const;
};

inline XML
operator""_xml (const char *src, const size_t src_len)
{
  return XML::parse (src);
}

}

#endif // SIMPLE_XML_H
