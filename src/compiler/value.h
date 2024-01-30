#ifndef COMPILER_VALUE_H_
#define COMPILER_VALUE_H_

typedef int Value;

typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

void InitValueArray(ValueArray* array);
void PushBackValueArray(ValueArray* array, Value value);
void FreeValueArray(ValueArray* array);
void PrintValue(Value value);

#endif