/**
* \file PrtValues.h
* \brief Functions and data structures for P values.
* Every P value is a combination of a type expression and an in-memory representation
* of the value. The type expression cannot be `any`, but `any` may appear as an inner type.
* Here are some examples:
*
* `true : bool`, `1 : int`, `null : event`, `null : model`, `(1, 2) : (any, int)`.
*
* The type a value does not change under mutation, and the inner-types of an expression
* constrain the legal mutations of a value. For instance, it is illegal to update `(1, 2) : (any, int)` to become
* `(1, true) : (any, int)` because the second element in the tuple must be an integer.
* Also note that values contained in complex data types may have more specific type information. For instance,
* the value obtained by reading the first element of `(1, true) : (any, int)` is `1 : int`.
*/
#ifndef PRTVALUES_H
#define PRTVALUES_H
#include "PrtTypes.h"

#ifdef __cplusplus
extern "C"{
#endif

/** The null values (null, event, machine, model) are represented by this value. 
*/
#define PRT_CNST_NULL 0

/** The halt event will always have this id.
*/
#define PRT_CNST_HALT 1

/**
* \enum
* These are the kinds of values in the P type system.
**/
typedef enum PRT_VALUE_KIND
{
	PRT_VALKIND_PRIM = 0,     /**< The kind of a primitive value        */
	PRT_VALKIND_FORGN = 1,    /**< The kind of all foreign values       */
	PRT_VALKIND_TUPLE = 2,    /**< The kind of all (named) tuple values */
	PRT_VALKIND_SEQ = 3,      /**< The kind of all sequence values      */
	PRT_VALKIND_MAP = 4,      /**< The kind of all map values           */
	PRT_VALUE_KIND_COUNT = 5, /**< The number of value kinds            */
} PRT_VALUE_KIND;

/**
* \enum
* These are the kinds of primitive values in the P type system.
**/
typedef enum PRT_PRIMVALUE_KIND
{
	PRT_PRIMVALKIND_BOOL = 0,        /**< The kind bool values      */
	PRT_PRIMVALKIND_EVENT = 1,       /**< The kind of event values */
	PRT_PRIMVALKIND_INT = 2,         /**< The kind of int values */
	PRT_PRIMVALKIND_MACHINE = 3,     /**< The kind of machine values */
	PRT_PRIMVALKIND_MODEL = 4,       /**< The kind of model values */
	PRT_PRIMVALKIND_NULL = 5,	     /**< The kind of null values (no field in union) */
	PRT_PRIMVALUE_KIND_COUNT = 6,    /**< The number of type kinds        */
} PRT_PRIMVALUE_KIND;

/** A Union type to discriminate the Prt value */
typedef struct PRT_VALUE {
	PRT_TYPE *type;               /**< the type of this value */
	PRT_VALUE_KIND discriminator; /**< A value kind to discriminate the union */
	union
	{
		struct PRT_PRIMVALUE *prim;
		struct PRT_FORGNVALUE *frgn;	/**< A foreign value       */
		struct PRT_TUPVALUE *tuple;		/**< A (named) tuple value */
		struct PRT_SEQVALUE *seq;		/**< A sequence value	   */
		struct PRT_MAPVALUE *map;		/**< A map value		   */
	} valueUnion;
} PRT_VALUE;

typedef struct PRT_PRIMVALUE
{
	PRT_PRIMVALUE_KIND discriminator; /**< A prim value kind to discriminate the union */
	union {
		PRT_BOOLEAN bl;   /**< A boolean value.    */
		PRT_UINT32  ev;   /**< An event id.        */
		PRT_INT32   nt;   /**< A signed integer.   */
		PRT_UINT32  mc;   /**< A machine machine        */
		PRT_UINT32  md;   /**< A model machine id. */
	} value;              /**< A primitive value is one of the above. */
} PRT_PRIMVALUE;

/** A foreign value is foreign type paired with a void *. */
typedef struct PRT_FORGNVALUE
{
	void    *value;   /**< A pointer to the foreign value. */
} PRT_FORGNVALUE;

/** A tuple value is a (named) tuple represented as an array. */
typedef struct PRT_TUPVALUE
{
	PRT_VALUE **values;   /**< Is an array of tuple args.    */
} PRT_TUPVALUE;

/** A sequence value is a sequence represented as a dynamic array. */
typedef struct PRT_SEQVALUE
{
	PRT_UINT32 size;     /**< The number of elements in the sequence. */
	PRT_UINT32 capacity; /**< The number of elements before resizing  */
	PRT_VALUE **values;  /**< An array of values in the sequence.     */
} PRT_SEQVALUE;

/** A map value is represented as a hash-table. */
typedef struct PRT_MAPVALUE
{
	PRT_UINT32   size;       /**< The number of elements in the map.      */
	PRT_UINT32   capNum;     /**< An opaque number related to the number of buckets */
	struct PRT_MAPNODE *first;    /**< First element inserted into the map. */
	struct PRT_MAPNODE *last;     /**< Last element inserted into the map. */
	struct PRT_MAPNODE **buckets; /**< An array of pointers to chained nodes.  */
} PRT_MAPVALUE;

/** A key-value node of a map. */
typedef struct PRT_MAPNODE
{
	PRT_VALUE *key;     /**< The key of this node. */
	PRT_VALUE *value;   /**< The value of this node. */
	struct PRT_MAPNODE *bucketNext;   /**< The next node in this bucket.         */
	struct PRT_MAPNODE *insertNext;   /**< The next node in insertion order.     */
	struct PRT_MAPNODE *insertPrev;   /**< The previous node in insertion order. */
} PRT_MAPNODE;

/** Makes a default value of an abitrary type. The defaults (def) are as follows:
* 1.  def(null)                = `null : null`.
* 2.  def(any)                 = `null : null`.
* 3.  def(bool)                = `false : bool`.
* 4.  def(event)               = `null : event`.
* 5.  def(foreign)             = `NULL pointer : foreign`.
* 6.  def(machine)             = `null : machine`.
* 7.  def(int)                 = `0 : int`.
* 8.  def(map[S, T])           = `[] : map[S, T]`.
* 9.  def(model)               = `null : model`.
* 10. def((l1: S1,.., ln: Sn)) = `(l1 = def(S1),..., ln = def(Sn)) : (l1: S1,..., ln: Sn)`.
* 11. def([S])                 = `[] : [S]`.
* 12. def((S1,..,Sn))          = `(def(S1),..., def(S2)) : (S1,..., Sn)`.
* @param[in] type A type expression (will be cloned).
* @returns The default value of the type. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkDefaultValue(_In_ PRT_TYPE *type);

/** Makes a boolean value.
* @param[in] value A boolean value.
* @returns A proper boolean value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkBoolValue(_In_ PRT_BOOLEAN value);

/** Makes an event value.
* @param[in] value An event id.
* @returns A proper event value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkEventValue(_In_ PRT_UINT32 value);

/** Makes an integer value.
* @param[in] value An int value.
* @returns A proper int value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkIntValue(_In_ PRT_INT32 value);

/** Makes null value.
* @returns A proper null value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkNullValue();

/** Makes an machine value.
* @param[in] value A machine id.
* @returns A propermachinevalue. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkMachineValue(_In_ PRT_UINT32 value);

/** Makes an model value.
* @param[in] value A model machine id.
* @returns A proper model value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkModelValue(_In_ PRT_UINT32 value);

/** Makes a foreign value.
* @param[in] type A foreign type (will be cloned).
* @param[in] value A pointer to foreign data (will be cloned).
* @returns A proper foreign value. Caller is responsible for freeing.
* @see PrtFreeValue
*/
PRT_VALUE *PrtMkForeignValue(_In_ PRT_TYPE *type, _In_ void *value);

/** Sets the value of a boolean.
* @param[in,out] prmVal A primitive boolean value to mutate.
* @param[in]     value The value to set.
*/
void PrtPrimSetBool(_Inout_ PRT_VALUE *prmVal, _In_ PRT_BOOLEAN value);

/** Gets the value of a boolean.
* @param[in] prmVal A primitive boolean value.
* @returns A boolean.
*/
PRT_BOOLEAN PrtPrimGetBool(_In_ PRT_VALUE *prmVal);

/** Sets the value of an event.
* @param[in,out] prmVal A primitive event value to mutate.
* @param[in]     value The value to set.
*/
void PrtPrimSetEvent(_Inout_ PRT_VALUE *prmVal, _In_ PRT_UINT32 value);

/** Gets the value of an event.
* @param[in] prmVal A primitive event value.
* @returns An event id.
*/
PRT_UINT32 PrtPrimGetEvent(_In_ PRT_VALUE *prmVal);

/** Sets the value of an int.
* @param[in,out] prmVal A primitive int value to mutate.
* @param[in]     value The value to set.
*/
void PrtPrimSetInt(_Inout_ PRT_VALUE *prmVal, _In_ PRT_INT32 value);

/** Gets the value of an integer.
* @param[in] prmVal A primitive int value.
* @returns An integer.
*/
PRT_INT32 PrtPrimGetInt(_In_ PRT_VALUE *prmVal);

/** Sets the value of an Machine.
* @param[in,out] prmVal A primitivemachinevalue to mutate.
* @param[in]     value The value to set.
*/
void PrtPrimSetMachine(_Inout_ PRT_VALUE *prmVal, _In_ PRT_UINT32 value);

/** Gets the value of an Machine.
* @param[in] prmVal A primitive machine value.
* @returns A machine machine
*/
PRT_UINT32 PrtPrimGetMachine(_In_ PRT_VALUE *prmVal);

/** Sets the value of an model.
* @param[in,out] prmVal A primitive model value to mutate.
* @param[in]     value The value to set.
*/
void PrtPrimSetModel(_Inout_ PRT_VALUE *prmVal, _In_ PRT_UINT32 value);

/** Gets the value of an model.
* @param[in] prmVal A primitive model value.
* @returns A model machine machine
*/
PRT_UINT32 PrtPrimGetModel(_In_ PRT_VALUE *prmVal);

/** Sets an element in a (named) tuple by index.
* @param[in,out] tuple A (named) tuple to mutate.
* @param[in]     index A 0-based element index.
* @param[in]     value The value to set (will be cloned).
*/
void PrtTupleSet(_Inout_ PRT_VALUE *tuple, _In_ PRT_UINT32 index, _In_ PRT_VALUE *value);

/** Gets an element in a (named) tuple by index.
* @param[in] tuple A (named) tuple.
* @param[in] index A 0-based element index.
* @returns The element at index i. Caller is responsible for freeing.
*/
PRT_VALUE* PrtTupleGet(_In_ PRT_VALUE *tuple, _In_ PRT_UINT32 index);

/** Sets an element in a named tuple by name.
* @param[in,out] tuple A named tuple to mutate.
* @param[in]     name  The name of the element to set.
* @param[in]     value The value to set (will be cloned).
*/
void PrtNmdTupleSet(_Inout_ PRT_VALUE *tuple, _In_ PRT_STRING name, _In_ PRT_VALUE *value);

/** Gets an element in a named tuple by name.
* @param[in] tuple A named tuple.
* @param[in] name  The name of the element to set.
* @returns The element named name. Caller is responsible for freeing.
*/
PRT_VALUE* PrtNmdTupleGet(_In_ PRT_VALUE *tuple, _In_ PRT_STRING name);

/** Updates the sequence at index.
* @param[in,out] seq   A sequence to mutate.
* @param[in]     index The name of the element to set. A value must already exist at this index.
* @param[in]     value The value to store at index (will be cloned).
*/
void PrtSeqUpdate(_Inout_ PRT_VALUE *seq, _In_ PRT_UINT32 index, _In_ PRT_VALUE *value);

/** Inserts value into the sequence at index.
* Index cannot be larger than the size of the sequence. Insertion causes:
* seq'[index] = value.
* For all i >= index, if seq[i] is defined, then seq'[i + 1] = seq[i].
* For all i < index, if seq[i] is defined, then seq'[i] = seq[i].
* @param[in,out] seq   A sequence to mutate.
* @param[in]     index An 0-based index s.t. 0 <= index <= size(seq).
* @param[in]     value The value to store at index (will be cloned).
*/
void PrtSeqInsert(_Inout_ PRT_VALUE *seq, _In_ PRT_UINT32 index, _In_ PRT_VALUE* value);

/** Removes the value at index from the sequence, and shortens the sequence by one.
* seq[index] must be defined. Removal causes:
* For all i > index, if seq[i] is defined, then seq'[i - 1] = seq[i].
* For all i < index, if seq[i] is defined, then seq'[i] = seq[i].
* @param[in,out] seq   A sequence to mutate.
* @param[in]     index An 0-based index s.t. 0 <= index < size(seq).
*/
void PrtSeqRemove(_Inout_ PRT_VALUE *seq, _In_ PRT_UINT32 index);

/** Gets the sequence at index.
* @param[in] seq   A sequence.
* @param[in] index A 0-based index s.t. 0 <= index < size(seq).
* @returns The value at index. Caller is responsible for freeing.
*/
PRT_VALUE* PrtSeqGet(_In_ PRT_VALUE *seq, _In_ PRT_UINT32 index);

/** Gets the size of a sequence.
* @param[in] seq A sequence.
* @returns The size of the sequence.
*/
PRT_UINT32 PrtSeqSizeOf(_In_ PRT_VALUE *seq);

/** Updates the map at key.
* If key is not in the map, then adds it.
* If key is already in the map, then changes its mapping.
* @param[in,out] map   A map to mutate.
* @param[in]     key   The key to update (will be cloned).
* @param[in]     value The value to which the key maps (will be cloned).
*/
void PrtMapUpdate(_Inout_ PRT_VALUE *map, _In_ PRT_VALUE* key, _In_ PRT_VALUE* value);

/** Updates the map at key.
* If key is not in the map, then adds it.
* If key is already in the map, then changes its mapping.
* @param[in,out] map   A map to mutate.
* @param[in]     key   The key to update (will be cloned if cloneKeyVals is true).
* @param[in]     value The value to which the key maps (will be cloned if cloneKeyVals is true).
* @param[in]     cloneKeyVals Only set to false if key/value will be forever owned by this map.
*/
void PrtMapUpdateEx(_Inout_ PRT_VALUE *map, _In_ PRT_VALUE *key, _In_ PRT_VALUE *value, _In_ PRT_BOOLEAN cloneKeyVals);

/** Remove the key from the map.
* If the key is not in then map, then the map is unchanged.
* @param[in,out] map   A map to mutate.
* @param[in]     key   The key to remove.
*/
void PrtMapRemove(_Inout_ PRT_VALUE *map, _In_ PRT_VALUE *key);

/** Gets the value to which this key maps.
* The key must be present the map.
* @param[in] map A map.
* @param[in] key The key to lookup.
* @returns The value to which the key maps. Caller is responsible for freeing.
*/
PRT_VALUE *PrtMapGet(_In_ PRT_VALUE *map, _In_ PRT_VALUE* key);

/** Converts a map to sequence of keys. Keys are returned in insertion order.
* @param[in] map A map.
* @returns The sequence of its keys. Caller is responsible for freeing.
*/
PRT_VALUE *PrtMapGetKeys(_In_ PRT_VALUE *map);

/** Converts a map to sequence of values. values are returned in insertion order.
* @param[in] map A map.
* @returns The sequence of its values (map image). Caller is responsible for freeing.
*/
PRT_VALUE *PrtMapGetValues(_In_ PRT_VALUE *map);

/** Returns true if the map contains key; false otherwise.
* @param[in] map A map.
* @param[in] key The key to lookup.
* @returns Returns true if the map contains key; false otherwise.
*/
PRT_BOOLEAN PrtMapExists(_In_ PRT_VALUE *map, _In_ PRT_VALUE *key);

/** Gets the size of a map.
* @param[in] map A map.
* @returns The size of the map.
*/
PRT_UINT32 PrtMapSizeOf(_In_ PRT_VALUE *map);

/** The hypothetical maximum number of keys that could be accessed in constant-time.
* @param[in] map A map.
* @returns The capacity of the map.
*/
PRT_UINT32 PrtMapCapacity(_In_ PRT_VALUE *map);

/** Determines if value inhabits type.
* @param[in] value The value to check.
* @param[in] type  The type to check.
* @returns `true` if value inhabits type, `false` otherwise.
*/
PRT_BOOLEAN PrtInhabitsType(_In_ PRT_VALUE* value, _In_ PRT_TYPE *type);

/** Determines if value is a null event, id, or model.
* @param[in] value The value to check.
* @returns `true` if the null value, `false` otherwise.
*/
PRT_BOOLEAN PrtIsNullValue(_In_ PRT_VALUE* value);

/** Casts value to type.
* Caller must know that type cast will succeed.
* Method checks validity of type cast and causes an assertion failure if the case is invalid.
* @param[in] value The value to cast (will be cloned).
* @param[in] type  The type to cast (will be cloned).
* @returns A copy of value that is a subtype of type. Caller is responsible for freeing.
*/
PRT_VALUE *PrtCastValue(_In_ PRT_VALUE* value, _In_ PRT_TYPE *type);

/** Returns a hash of this value.
* @param[in] value The value to hash.
* @returns The hash code.
*/
PRT_UINT32 PrtGetHashCodeValue(_In_ PRT_VALUE *value);

/** Returns `true` if values are equivalent; `false` otherwise.
* @param[in] value1 The first value.
* @param[in] value2 The second value.
* @returns `true` if values are equivalent; `false` otherwise.
*/
PRT_BOOLEAN PrtIsEqualValue(_In_ PRT_VALUE *value1, _In_ PRT_VALUE *value2);

/** Deeply clones a value.
* @param[in] value The value to clone.
* @returns The cloned value. Caller is responsible for freeing.
*/
PRT_VALUE *PrtCloneValue(_In_ PRT_VALUE *value);

/** Recursively frees a value. Should only be called on values created using PrtMkXValue()
* @param[in,out] value The value to free.
*/
void PrtFreeValue(_Inout_ PRT_VALUE *value);

/** Shallow test that value members are non-null.
* @param[in] value The value to check.
*/
PRT_BOOLEAN PrtIsValidValue(_In_ PRT_VALUE *value);

#ifdef __cplusplus
}
#endif
#endif