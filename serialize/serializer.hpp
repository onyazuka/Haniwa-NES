#pragma once
#include <functional>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <deque>
#include <memory.h>

// using void_t with all template arguments as void
template< class... >
using void_t = void;

// checks for operator<<
template<typename T, typename = void>
struct HasOutputOperator : std::false_type {};

template<typename T>
struct HasOutputOperator<T, void_t<decltype(std::declval<T>().operator<<)>> : std::true_type {};


// type checks
template<typename T, typename = void>
struct IsIterable : std::false_type {};

template <typename T>
struct IsIterable<T, void_t<typename T::iterator>> : std::true_type {};


template<typename T>
struct IsVector : std::false_type {};

template<typename T, typename... Others>
struct IsVector<std::vector<T, Others...>> : std::true_type{};

template<typename T>
struct IsList : std::false_type {};

template<typename T, typename... Others>
struct IsList<std::list<T, Others...>> : std::true_type{};

template<typename T>
struct IsDeque : std::false_type {};

template<typename T, typename... Others>
struct IsDeque<std::deque<T, Others...>> : std::true_type{};

template<typename T>
struct IsMap : std::false_type {};

template<typename T, typename... Others>
struct IsMap<std::map<T, Others...>> : std::true_type{};

template<typename T>
struct IsMultimap : std::false_type {};

template<typename T, typename... Others>
struct IsMultimap<std::multimap<T, Others...>> : std::true_type{};

template<typename T>
struct IsUndorderedMap : std::false_type {};

template<typename T, typename... Others>
struct IsUndorderedMap<std::unordered_map<T, Others...>> : std::true_type{};

template<typename T>
struct IsUndorderedMultimap : std::false_type {};

template<typename T, typename... Others>
struct IsUndorderedMultimap<std::unordered_multimap<T, Others...>> : std::true_type{};

template<typename T>
struct IsSet : std::false_type {};

template<typename T, typename... Others>
struct IsSet<std::set<T, Others...>> : std::true_type{};

template<typename T>
struct IsMultiset : std::false_type {};

template<typename T, typename... Others>
struct IsMultiset<std::multiset<T, Others...>> : std::true_type{};

template<typename T>
struct IsUnorderedSet : std::false_type {};

template<typename T, typename... Others>
struct IsUnorderedSet<std::unordered_set<T, Others...>> : std::true_type{};

template<typename T>
struct IsUnorderedMultiset : std::false_type {};

template<typename T, typename... Others>
struct IsUnorderedMultiset<std::unordered_multiset<T, Others...>> : std::true_type{};


template<typename T>
struct IsString : std::false_type {};

template<>
struct IsString<std::string> : std::true_type{};


/*
    Not usual serializator, it works as:
        - writes data in passed string buffer
        - Supported data types:
            - arithmetic(numeric)
            - standart containers
            - user types
        - User types must inherit Serializable class and provide needed interface.
        WARNING! Now it accepts pointers and writes its values in buffer. So, maybe, it is good only for values now, but can be extended.
        WARNING! To add functions to some else standart type, you should specify Serializer::SerializeUnit template function for this type.
*/
namespace Serialization
{

    typedef uint64_t BytesCount;
    typedef uint64_t ElementsCount;
    typedef uint64_t SerializerId;
    typedef std::function<BytesCount(std::string&)> SerializerFunction;
    typedef std::function<BytesCount(const std::string&, BytesCount)> DeserializerFunction;

    class SerializerExceptions
    {
    public:
        struct InvalidType : std::runtime_error
        {
            InvalidType(const std::string& err);
        };
        struct InvalidArgs : std::runtime_error
        {
            InvalidArgs(const std::string& err);
        };
    };


    /*
        Contains template structs that help serialize std containers.
    */
    class ContainerSerializerHelper
    {
        template<typename T, typename Check = void>
        struct ContainerAdder;
    };

    template<typename T>
    struct ContainerSerializerHelper::ContainerAdder<T, std::enable_if_t<IsVector<T>::value || IsList<T>::value || IsDeque<T>::value>>
    {
        template<typename Val>
        static void addTo(T* cont, Val* val)
        {
            cont->push_back(*val);
        }
    };

    template<typename T>
    struct ContainerSerializerHelper::ContainerAdder<T, std::enable_if_t<IsMap<T>::value || IsMultimap<T>::value ||
                                                                         IsUndorderedMap<T>::value || IsUndorderedMultimap<T>::value ||
                                                                         IsSet<T>::value || IsMultiset<T>::value ||
                                                                         IsUnorderedSet<T>::value || IsUnorderedMultiset<T>::value>>
    {
        template<typename Val>
        // 'val' is std::pair here
        static void addTo(T* cont, Val* val)
        {
            cont->insert(*val);
        }
    };


    /*
        Array serialization helper.
        To array be serialized correctly, it MUST be wrapped in this structure.
    */
    template<typename T>
    struct ArrayWrapper
    {
        ArrayWrapper()
            : start{nullptr}, size{0} {}
        ArrayWrapper(T* _start, ElementsCount _size)
            : start{_start}, size{_size} {}
        T* start;
        ElementsCount size;
    };


    /*
        This class must be inherited if object wants to be serializable and has single serializer;
    */
    class Serializable
    {
    public:
        virtual BytesCount serialize(std::string& buf) = 0;
    };

    /*
        This class must be inherited if object wants to be deserializable and has single serializer;
    */
    class Deserializable
    {
        virtual BytesCount deserialize(const std::string& buf, BytesCount offset) = 0;
    };

    /*
        This class must be inherited if object wants to be serializable and has multiple serializers;
        This object should have map of serializers, from which needed serializer will be selected by 'id'.
    */
    class MultipleSerializable
    {
    public:
        typedef std::unordered_map<SerializerId, SerializerFunction> SerializersMap;

        inline BytesCount serialize(std::string& buf) { return getCurrentSerializer()(buf); }
        void registerSerializer(SerializerId _id, const SerializerFunction& f);
        // throws if not have such Id
        const SerializerFunction& getCurrentSerializer();
        inline void setSerializerId(SerializerId _id) { curId = _id; }
        inline SerializerId getSerializerId() const { return curId; }


    private:
        SerializersMap smap;
        SerializerId curId;
    };

    /*
        This class must be inherited if object wants to be deserializable and has multiple serializers;
        This object should have map of deserializers, from which needed deserializer will be selected by 'id'.
    */
    class MultipleDeserializable
    {
    public:
        typedef std::unordered_map<SerializerId, DeserializerFunction> DeserializersMap;

        inline BytesCount deserialize(const std::string& buf, BytesCount offset) { return getCurrentDeserializer()(buf, offset); }
        void registerDeserializer(SerializerId _id, const DeserializerFunction& f);
        // throws if not have such Id
        const DeserializerFunction& getCurrentDeserializer();

        inline void setDeserializerId(SerializerId _id) { curId = _id; }

        inline SerializerId getDeserializerId() const { return curId; }

    private:
        DeserializersMap dsmap;
        SerializerId curId;
    };


    // checks for custorm type
    template<typename T, typename = void>
    struct IsSerializable : std::false_type {};

    template<typename T>
    struct IsSerializable<T, std::enable_if_t<std::is_base_of<Serializable, T>::value>> : std::true_type {};

    template<typename T, typename = void>
    struct IsDeserializable : std::false_type {};

    template<typename T>
    struct IsDeserializable<T, std::enable_if_t<std::is_base_of<Deserializable, T>::value>> : std::true_type {};

    template<typename T, typename = void>
    struct IsMultipleSerializable : std::false_type {};

    template<typename T>
    struct IsMultipleSerializable<T, std::enable_if_t<std::is_base_of<MultipleSerializable, T>::value>> : std::true_type {};

    template<typename T, typename = void>
    struct IsMultipleDeserializable : std::false_type {};

    template<typename T>
    struct IsMultipleDeserializable<T, std::enable_if_t<std::is_base_of<MultipleDeserializable, T>::value>> : std::true_type {};


    class Serializer
    {
    public:
        template<typename T, typename Check = void>
        struct SerializeUnit;

        template<typename Arg>
        static BytesCount serializeAll(std::string& buf, Arg* data);

        template<typename Arg, typename... Args>
        static BytesCount serializeAll(std::string& buf, Arg* data, Args... args);
    };

    /*
        End of recursion
    */
    template<typename Arg>
    BytesCount Serializer::serializeAll(std::string& buf, Arg* data)
    {
        return Serializer::SerializeUnit<Arg>::serializeUnit(buf, data);
    }

    /*
        Serializes all arguments passed. Writes as binary data in 'buf'
    */
    template<typename Arg, typename... Args>
    BytesCount Serializer::serializeAll(std::string& buf, Arg* data, Args... args)
    {
        return Serializer::SerializeUnit<Arg>::serializeUnit(buf, data) + serializeAll(buf, args...);
    }


    /*
        Serializer for arithmetic types.
    */
    template<typename T>
    struct Serializer::SerializeUnit<T, std::enable_if_t<std::is_arithmetic<T>::value> >
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, T* data)
        {
            int initSize = buf.size();
            buf.resize(buf.size() + sizeof(T));
            memcpy(&buf[initSize], data, sizeof(T));
            return sizeof(T);
        }
    };

    /*
        Serializer for array wrappers.
    */
    template<typename T>
    struct Serializer::SerializeUnit<ArrayWrapper<T>>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, ArrayWrapper<T>* data)
        {
            if(data->start == nullptr || data->size <= 0) throw SerializerExceptions::InvalidArgs{"serializeUnit<ArrayWrapper>() - invalid arguments passed"};
            BytesCount written = 0;
            written += SerializeUnit<ElementsCount>::serializeUnit(buf, &(data->size));
            for(ElementsCount i = 0; i < data->size; ++i)
            {
                written += SerializeUnit<T>::serializeUnit(buf, data->start + i);
            }
            return written;
        }
    };

    /*
        Serializer for iterables(they are in most cases can be serialized in the same way).
    */
    template<typename T>
    struct Serializer::SerializeUnit<T, std::enable_if_t<IsIterable<T>::value>>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, T* data)
        {
            using ContSize = typename T::size_type;
            using ValueType = typename T::value_type;
            BytesCount dataSize = 0;
            ContSize sz = data->size();
            // writing size
            dataSize += SerializeUnit<ContSize>::serializeUnit(buf, &sz);
            auto iter = data->begin();
            // writing 'size' parts of data
            while(iter != data->end())
            {
                // container value type can be const, that should not interfere serialization
                // and several containers(such as std::set) always points to const types
                typedef std::remove_const_t<ValueType> NonConstValueType;
                dataSize += SerializeUnit<ValueType>::serializeUnit(buf, const_cast<NonConstValueType*>(&(*iter)));
                ++iter;
            }
            return dataSize;
        }
    };



    /*
        Serializer for strings.
        They are, of course, iterable, but it is more efficient to write it in one operation, because data is stores sequentially in memory.
    */
    template<>
    struct Serializer::SerializeUnit<std::string>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, std::string* data)
        {
            using ContSize = typename std::string::size_type;
            BytesCount dataSize = 0;
            ContSize sz = data->size();
            // writing size
            dataSize += SerializeUnit<ContSize>::serializeUnit(buf, &sz);
            BytesCount initSize = buf.size();
            buf.resize(buf.size() + data->size());
            // writing 'size' characters
            memcpy(&buf[initSize], &((*data)[0]), data->size());
            dataSize += data->size();
            return dataSize;
        }
    };

    /*
        Serializer for user type that inherits IsSerializable
    */
    template<typename T>
    struct Serializer::SerializeUnit<T, std::enable_if_t<IsSerializable<T>::value>>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, T* data)
        {
            return data->serialize(buf);
        }
    };


    /*
        Serializer for user type that inherits IsMultipleSerializable
    */
    template<typename T>
    struct Serializer::SerializeUnit<T, std::enable_if_t<IsMultipleSerializable<T>::value>>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, T* data)
        {
            return data->serialize(buf);
        }
    };

    /*
        Serializer for std::pair
    */
    template<typename T1, typename T2>
    struct Serializer::SerializeUnit<std::pair<T1, T2>>
    {
        SerializeUnit() = default;

        static BytesCount serializeUnit(std::string& buf, std::pair<T1, T2>* data)
        {
            typedef std::remove_const_t<T1> NonConstT1;
            // throwing away const, because std containers, such as std::unordered_map, use const Key in their value type
            return serializeAll(buf, const_cast<NonConstT1*>(&data->first), &data->second);
        }
    };


    /*
        Deserializer works in the same way as serializer,
        but for custom types inheritance from Deserializable must be done.
    */
    class Deserializer
    {
    public:

        template<typename T, typename Check = void>
        struct DeserializeUnit;

        template<typename Arg>
        static BytesCount deserializeAll(const std::string& buf, BytesCount offset, Arg* data);

        template<typename Arg, typename... Args>
        static BytesCount deserializeAll(const std::string& buf, BytesCount offset, Arg* data, Args... args);
    };

    template<typename Arg>
    BytesCount Deserializer::deserializeAll(const std::string& buf, BytesCount offset, Arg* data)
    {
        return Deserializer::DeserializeUnit<Arg>::deserializeUnit(buf, offset, data);
    }

    template<typename Arg, typename... Args>
    BytesCount Deserializer::deserializeAll(const std::string& buf, BytesCount offset, Arg* data, Args... args)
    {
        BytesCount nextOffset = Deserializer::DeserializeUnit<Arg>::deserializeUnit(buf, offset, data);
        return nextOffset + deserializeAll(buf, offset + nextOffset, args...);
    }


    // for arithmetic
    template<typename T>
    struct Deserializer::DeserializeUnit<T, std::enable_if_t<std::is_arithmetic<T>::value> >
    {
        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, T* data)
        {
            memcpy(data, &buf[offset], sizeof(T));
            return sizeof(T);
        }
    };


    /*
        Deserializer for array wrappers.
        WARNING: before deserialization, array wrapper should have 'start' member set(memory allocated).
    */
    template<typename T>
    struct Deserializer::DeserializeUnit<ArrayWrapper<T>>
    {
        DeserializeUnit() = default;

        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, ArrayWrapper<T>* data)
        {
            if(data->start == nullptr) throw SerializerExceptions::InvalidArgs{"serializeUnit<ArrayWrapper>() - invalid arguments passed"};
            BytesCount read = 0;
            BytesCount internalOffset = offset;
            read += DeserializeUnit<ElementsCount>::deserializeUnit(buf, internalOffset, &(data->size));
            internalOffset += sizeof(ElementsCount);
            for(ElementsCount i = 0; i < data->size; ++i)
            {
                read += DeserializeUnit<T>::deserializeUnit(buf, internalOffset, data->start + i);
                internalOffset += sizeof(T);
            }
            return read;
        }
    };



    // for vector, list, deque
    template<typename T>
    struct Deserializer::DeserializeUnit<T, std::enable_if_t<IsIterable<T>::value>>
    {
        static BytesCount deserializeUnit(const std::string &buf, BytesCount offset, T *data)
        {
            data->clear();
            using ContSize = typename T::size_type;
            using DataType = typename T::value_type;
            ContSize contSize;
            BytesCount internalOffset = offset;
            internalOffset += DeserializeUnit<ContSize>::deserializeUnit(buf, internalOffset, &contSize);
            for(ContSize i = 0; i < contSize; ++i)
            {
                DataType dataPiece;
                internalOffset += DeserializeUnit<DataType>::deserializeUnit(buf, internalOffset, &dataPiece);
                ContainerSerializerHelper::ContainerAdder<T>::addTo(data, &dataPiece);
            }
            return internalOffset - offset;
        }
    };


    // for std::string
    template<>
    struct Deserializer::DeserializeUnit<std::string>
    {
        DeserializeUnit() = default;

        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, std::string* data)
        {
            data->clear();
            using ContSize = typename std::string::size_type;
            ContSize contSize;
            BytesCount internalOffset = offset;
            internalOffset += DeserializeUnit<ContSize>::deserializeUnit(buf, internalOffset, &contSize);
            data->resize(contSize);
            memcpy(&((*data)[0]), &buf[internalOffset], contSize);
            internalOffset += contSize;
            return internalOffset - offset;
        }
    };


    // for user type
    template<typename T>
    struct Deserializer::DeserializeUnit<T, std::enable_if_t<IsDeserializable<T>::value>>
    {
        DeserializeUnit() = default;

        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, T* data)
        {
            return data->deserialize(buf, offset);
        }
    };


    // for user type with multiple serializators
    template<typename T>
    struct Deserializer::DeserializeUnit<T, std::enable_if_t<IsMultipleDeserializable<T>::value>>
    {
        DeserializeUnit() = default;

        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, T* data)
        {
            return data->deserialize(buf, offset);
        }
    };


    /*
        std::pair
    */
    template<typename T1, typename T2>
    struct Deserializer::DeserializeUnit<std::pair<T1, T2>>
    {
        DeserializeUnit() = default;

        static BytesCount deserializeUnit(const std::string& buf, BytesCount offset, std::pair<T1, T2>* data)
        {
            typedef std::remove_const_t<T1> NonConstT1;
            // throwing away const, because std containers, such as std::unordered_map, use const Key in their value type
            return deserializeAll(buf, offset, const_cast<NonConstT1*>(&data->first), &data->second);
        }
    };

}


