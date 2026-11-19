#include "catalog.h"
#include <fstream>
#include <sstream>

namespace semantic {

void CatalogManager::addTable(const TableMetadata& table) {
    tables_[toLowerAscii(table.name)] = table;
}

bool CatalogManager::hasTable(const std::string& tableName) const {
    return tables_.find(toLowerAscii(tableName)) != tables_.end();
}

const TableMetadata* CatalogManager::getTable(const std::string& tableName) const {
    auto it = tables_.find(toLowerAscii(tableName));
    if (it == tables_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool CatalogManager::hasColumn(const std::string& tableName, const std::string& columnName) const {
    return getColumn(tableName, columnName) != nullptr;
}

const ColumnMetadata* CatalogManager::getColumn(const std::string& tableName, const std::string& columnName) const {
    const TableMetadata* table = getTable(tableName);
    if (!table) {
        return nullptr;
    }
    auto it = table->columnIndexByName.find(toLowerAscii(columnName));
    if (it == table->columnIndexByName.end()) {
        return nullptr;
    }
    return &table->orderedColumns[it->second];
}

std::vector<ColumnMetadata> CatalogManager::getOrderedColumns(const std::string& tableName) const {
    const TableMetadata* table = getTable(tableName);
    if (!table) {
        return {};
    }
    return table->orderedColumns;
}

// TODO (Persona 1): Extender o personalizar la carga de metadatos desde archivo
// y definir el archivo de esquema definitivo del equipo en feat/catalog-ast.
bool CatalogManager::loadFromCSV(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (firstLine) {
            firstLine = false;
            if (toLowerAscii(line).find("table") != std::string::npos) {
                continue; // saltar cabecera
            }
        }

        std::stringstream ss(line);
        std::string tableName, storagePath, colName, posStr, typeStr, pkStr, nullStr;
        if (!std::getline(ss, tableName, ',')) continue;
        if (!std::getline(ss, storagePath, ',')) continue;
        if (!std::getline(ss, colName, ',')) continue;
        if (!std::getline(ss, posStr, ',')) continue;
        if (!std::getline(ss, typeStr, ',')) continue;
        std::getline(ss, pkStr, ',');
        std::getline(ss, nullStr, ',');

        std::string key = toLowerAscii(tableName);
        if (tables_.find(key) == tables_.end()) {
            TableMetadata tbl;
            tbl.name = tableName;
            tbl.storagePath = storagePath;
            tables_[key] = tbl;
        }

        ColumnMetadata col;
        col.name = colName;
        col.tableName = tableName;
        col.ordinalPosition = std::stoi(posStr);
        col.type = stringToDataType(typeStr);
        col.isPrimaryKey = (toLowerAscii(pkStr) == "true" || pkStr == "1");
        col.isNullable = !(toLowerAscii(nullStr) == "false" || nullStr == "0");
        tables_[key].addColumn(col);
    }

    return !tables_.empty();
}

void CatalogManager::loadDefaultSchema() {
    clear();

    // Tabla 1: empleados
    TableMetadata empleados;
    empleados.name = "empleados";
    empleados.storagePath = "data/empleados.tbl";
    empleados.addColumn({"id", "empleados", 1, DataType::INT, true, false});
    empleados.addColumn({"nombre", "empleados", 2, DataType::VARCHAR, false, false});
    empleados.addColumn({"edad", "empleados", 3, DataType::INT, false, true});
    empleados.addColumn({"salario", "empleados", 4, DataType::FLOAT, false, false});
    empleados.addColumn({"activo", "empleados", 5, DataType::BOOLEAN, false, false});
    empleados.addColumn({"departamento_id", "empleados", 6, DataType::INT, false, true});
    addTable(empleados);

    // Tabla 2: departamentos
    TableMetadata departamentos;
    departamentos.name = "departamentos";
    departamentos.storagePath = "data/departamentos.tbl";
    departamentos.addColumn({"id", "departamentos", 1, DataType::INT, true, false});
    departamentos.addColumn({"nombre", "departamentos", 2, DataType::VARCHAR, false, false});
    departamentos.addColumn({"presupuesto", "departamentos", 3, DataType::FLOAT, false, false});
    departamentos.addColumn({"activo", "departamentos", 4, DataType::BOOLEAN, false, false});
    addTable(departamentos);

    // Tabla 3: proyectos
    TableMetadata proyectos;
    proyectos.name = "proyectos";
    proyectos.storagePath = "data/proyectos.tbl";
    proyectos.addColumn({"codigo", "proyectos", 1, DataType::INT, true, false});
    proyectos.addColumn({"titulo", "proyectos", 2, DataType::VARCHAR, false, false});
    proyectos.addColumn({"horas_estimadas", "proyectos", 3, DataType::INT, false, true});
    proyectos.addColumn({"costo", "proyectos", 4, DataType::FLOAT, false, false});
    proyectos.addColumn({"finalizado", "proyectos", 5, DataType::BOOLEAN, false, false});
    addTable(proyectos);
}

} // namespace semantic
