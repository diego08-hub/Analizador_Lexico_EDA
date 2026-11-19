#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "semantic_types.h"

namespace semantic {

// Gestor del Catálogo del Sistema (Tabla de Símbolos) con consultas O(1) promedio
class CatalogManager {
public:
    CatalogManager() = default;

    // Registra o reemplaza una tabla en el catálogo
    void addTable(const TableMetadata& table);

    // Consultas O(1) promedio sobre tablas y columnas (insensible a mayúsculas/minúsculas)
    bool hasTable(const std::string& tableName) const;
    const TableMetadata* getTable(const std::string& tableName) const;

    bool hasColumn(const std::string& tableName, const std::string& columnName) const;
    const ColumnMetadata* getColumn(const std::string& tableName, const std::string& columnName) const;

    // Retorna las columnas de una tabla en su orden ordinal (útil para expandir SELECT *)
    std::vector<ColumnMetadata> getOrderedColumns(const std::string& tableName) const;

    // Carga metadatos desde archivo CSV (formato: tabla,ruta,columna,posicion,tipo,pk,nullable)
    bool loadFromCSV(const std::string& filePath);

    // Carga un esquema de prueba en memoria (>= 3 tablas con distintos tipos)
    void loadDefaultSchema();

    size_t tableCount() const { return tables_.size(); }
    void clear() { tables_.clear(); }

private:
    std::unordered_map<std::string, TableMetadata> tables_;
};

} // namespace semantic
