t# Instruction Encode
## Описание

Данный проект реализует генерацию системы кодирования инструкций RISC-подобной архитектуры по входному JSON-файлу.

## Входные данные

JSON файл следующего вида:

- length — длина инструкции
- fields — список полей и их размеров
- instructions — описание форматов инструкций

Пример:

```json
{
  "length": "25",
  "fields": [
    { "R0": "5" },
    { "R1": "5" },
    { "R2": "5" },
    { "imm": ">=8" }
  ],
  "instructions": [
    {
      "insns": ["add", "sub"],
      "operands": ["R0", "R1"],
      "format": "alu"
    }
  ]
}
```

## Выходные данные

JSON массив инструкций:

- insn — имя инструкции
- fields — список полей с msb/lsb/value

## Архитектура

Проект разделён на модули:

- parser — чтение JSON
- validator — проверка входных данных
- allocator — построение битовой схемы
- generator — генерация выходного JSON
- utils — вспомогательные функции

## Сборка

```bash
cmake -S . -B build
cmake --build build
```

## Запуск

```bash
./build/instruction_encoder examples/input_example.json output.json
```

## Тесты

```bash
ctest --test-dir build --output-on-failure
```