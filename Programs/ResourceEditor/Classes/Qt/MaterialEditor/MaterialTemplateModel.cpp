#include "MaterialTemplateModel.h"

#include <Render/Material/FXCache.h>

MaterialTemplateModel::MaterialTemplateModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
}

MaterialTemplateModel::~MaterialTemplateModel()
{
}

void MaterialTemplateModel::SetSelectedMaterialType(DAVA::NMaterial::eType type)
{
    selectedMaterialType = type;

    invalidateFilter();
}

bool MaterialTemplateModel::filterAcceptsRow(int source_row, QModelIndex const& source_parent) const
{
    QModelIndex childIndex = sourceModel()->index(source_row, 0, source_parent);

    bool validType = false;
    int materialType = childIndex.data(Qt::UserRole + 1).toInt(&validType);

    if (!validType)
        return false;

    return (DAVA::NMaterial::eType(materialType) == selectedMaterialType);
}
