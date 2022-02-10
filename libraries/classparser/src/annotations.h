#pragma once
#include "classfile.h"
#include <map>
#include <vector>

namespace java
{
enum element_value_type : uint8_t
{
    INVALID = 0,
    STRING = 's',
    ENUM_CONSTANT = 'e',
    CLASS = 'c',
    ANNOTATION = '@',
    ARRAY = '[',            // one array dimension
    PRIMITIVE_INT = 'I',    // integer
    PRIMITIVE_BYTE = 'B',   // signed byte
    PRIMITIVE_CHAR = 'C',   // Unicode character code point in the Basic Multilingual Plane,
                            // encoded with UTF-16
    PRIMITIVE_DOUBLE = 'D', // double-precision floating-point value
    PRIMITIVE_FLOAT = 'F',  // single-precision floating-point value
    PRIMITIVE_LONG = 'J',   // long integer
    PRIMITIVE_SHORT = 'S',  // signed short
    PRIMITIVE_BOOLEAN = 'Z' // true or false
};
/**
 * The element_value structure is a discriminated union representing the value of an
 *element-value pair.
 * It is used to represent element values in all attributes that describe annotations
 * - RuntimeVisibleAnnotations
 * - RuntimeInvisibleAnnotations
 * - RuntimeVisibleParameterAnnotations
 * - RuntimeInvisibleParameterAnnotations).
 *
 * The element_value structure has the following format:
 */
class element_value
{
protected:
    element_value_type type;
    constant_pool &pool;

public:
    element_value(element_value_type type, constant_pool &pool) : type(type), pool(pool) {};
    virtual ~element_value() = default;

    element_value_type getElementValueType()
    {
        return type;
    }

    virtual std::string toString() = 0;

    static element_value *readElementValue(util::membuffer &input, constant_pool &pool);
};

/**
 * Each value of the annotations table represents a single runtime-visible annotation on a
 * program element.
 * The annotation structure has the following format:
 */
class annotation
{
public:
    using value_list = std::vector<std::pair<uint16_t, element_value *>>;

protected:
    /**
     * The value of the type_index item must be a valid index into the constant_pool table.
     * The constant_pool entry at that index must be a CONSTANT_Utf8_info (§4.4.7) structure
     * representing a field descriptor representing the annotation type corresponding
     * to the annotation represented by this annotation structure.
     */
    uint16_t type_index;
    /**
     * map between element_name_index and value.
     *
     * The value of the element_name_index item must be a valid index into the constant_pool
     *table.
     * The constant_pool entry at that index must be a CONSTANT_Utf8_info (§4.4.7) structure
     *representing
     * a valid field descriptor (§4.3.2) that denotes the name of the annotation type element
     *represented
     * by this element_value_pairs entry.
     */
    value_list name_val_pairs;
    /**
     * Reference to the parent constant pool
     */
    constant_pool &pool;

public:
    annotation(uint16_t type_index, constant_pool &pool)
        : type_index(type_index), pool(pool) {};
    ~annotation()
    {
        for (unsigned i = 0; i < name_val_pairs.size(); i++)
        {
            delete name_val_pairs[i].second;
        }
    }
    void add_pair(uint16_t key, element_value *value)
    {
        name_val_pairs.push_back(std::make_pair(key, value));
    }
    ;
    value_list::const_iterator begin()
    {
        return name_val_pairs.cbegin();
    }
    value_list::const_iterator end()
    {
        return name_val_pairs.cend();
    }
    std::string toString();
    static annotation *read(util::membuffer &input, constant_pool &pool);
};
using annotation_table = std::vector<annotation *>;

/// type for simple value annotation elements
class element_value_simple : public element_value
{
protected:
    /// index of the constant in the constant pool
    uint16_t index;

public:
    element_value_simple(element_value_type type, uint16_t index, constant_pool &pool)
        : element_value(type, pool), index(index) {
                                         // TODO: verify consistency
                                     };
    uint16_t getIndex()
    {
        return index;
    }
    std::string toString() override
    {
        return pool[index].toString();
    }
    ;
};
/// The enum_const_value item is used if the tag item is 'e'.
class element_value_enum : public element_value
{
protected:
    /**
     * The value of the type_name_index item must be a valid index into the constant_pool table.
     * The constant_pool entry at that index must be a CONSTANT_Utf8_info (§4.4.7) structure
     * representing a valid field descriptor (§4.3.2) that denotes the internal form of the
     * binary
     * name (§4.2.1) of the type of the enum constant represented by this element_value
     * structure.
     */
    uint16_t typeIndex;
    /**
     * The value of the const_name_index item must be a valid index into the constant_pool
     * table.
     * The constant_pool entry at that index must be a CONSTANT_Utf8_info (§4.4.7) structure
     * representing the simple name of the enum constant represented by this element_value
     * structure.
     */
    uint16_t valueIndex;

public:
    element_value_enum(element_value_type type, uint16_t typeIndex, uint16_t valueIndex,
                       constant_pool &pool)
        : element_value(type, pool), typeIndex(typeIndex), valueIndex(valueIndex)
    {
        // TODO: verify consistency
    }
    uint16_t getValueIndex()
    {
        return valueIndex;
    }
    uint16_t getTypeIndex()
    {
        return typeIndex;
    }
    std::string toString() override
    {
        return "enum value";
    }
    ;
};

class element_value_class : public element_value
{
protected:
    /**
     * The class_info_index item must be a valid index into the constant_pool table.
     * The constant_pool entry at that index must be a CONSTANT_Utf8_info (§4.4.7) structure
     * representing the return descriptor (§4.3.3) of the type that is reified by the class
     * represented by this element_value structure.
     *
     * For example, 'V' for Void.class, 'Ljava/lang/Object;' for Object, etc.
     *
     * Or in plain english, you can store type information in annotations. Yay.
     */
    uint16_t classIndex;

public:
    element_value_class(element_value_type type, uint16_t classIndex, constant_pool &pool)
        : element_value(type, pool), classIndex(classIndex)
    {
        // TODO: verify consistency
    }
    uint16_t getIndex()
    {
        return classIndex;
    }
    std::string toString() override
    {
        return "class";
    }
    ;
};

/// nested annotations... yay
class element_value_annotation : public element_value
{
private:
    annotation *nestedAnnotation;

public:
    element_value_annotation(element_value_type type, annotation *nestedAnnotation,
                             constant_pool &pool)
        : element_value(type, pool), nestedAnnotation(nestedAnnotation) {};
    ~element_value_annotation() override
    {
        if (nestedAnnotation)
        {
            delete nestedAnnotation;
            nestedAnnotation = nullptr;
        }
    }
    std::string toString() override
    {
        return "nested annotation";
    }
    ;
};

/// and arrays!
class element_value_array : public element_value
{
public:
    using elem_vec = std::vector<element_value *>;

protected:
    elem_vec values;

public:
    element_value_array(element_value_type type, std::vector<element_value *> &values,
                        constant_pool &pool)
        : element_value(type, pool), values(values) {};
    ~element_value_array() override
    {
        for (unsigned i = 0; i < values.size(); i++)
        {
            delete values[i];
        }
    }
    ;
    elem_vec::const_iterator begin()
    {
        return values.cbegin();
    }
    elem_vec::const_iterator end()
    {
        return values.cend();
    }
    std::string toString() override
    {
        return "array";
    }
    ;
};
}