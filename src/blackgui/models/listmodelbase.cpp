/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

// Drag and drop docu:
// http://doc.qt.io/qt-5/model-view-programming.html#using-drag-and-drop-with-item-views

#include "blackgui/models/columnformatters.h"
#include "blackgui/models/listmodelbase.h"
#include "blackgui/models/allmodelcontainers.h"
#include "blackgui/guiutility.h"
#include "blackmisc/compare.h"
#include "blackmisc/predicates.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/fileutils.h"
#include "blackmisc/sequence.h"
#include "blackmisc/variant.h"
#include "blackmisc/verify.h"
#include "blackmisc/worker.h"

#include <QFlags>
#include <QJsonDocument>
#include <QList>
#include <QMimeData>
#include <QtGlobal>
#include <QFileInfo>

using namespace BlackMisc;
using namespace BlackMisc::Aviation;

namespace BlackGui
{
    namespace Models
    {
        int CListModelBaseNonTemplate::columnCount(const QModelIndex &modelIndex) const
        {
            Q_UNUSED(modelIndex);
            int c = m_columns.size();
            return c;
        }

        QVariant CListModelBaseNonTemplate::headerData(int section, Qt::Orientation orientation, int role) const
        {
            if (orientation != Qt::Horizontal) { return QVariant(); }
            const bool handled = (role == Qt::DisplayRole || role == Qt::ToolTipRole || role == Qt::InitialSortOrderRole);
            if (!handled) { return QVariant(); }
            if (section < 0 || section >= m_columns.size()) { return QVariant(); }

            if (role == Qt::DisplayRole)
            {
                const QString header = m_columns.at(section).getColumnName(false);
                return QVariant(header);
            }
            if (role == Qt::ToolTipRole)
            {
                const QString header = m_columns.at(section).getColumnToolTip(false);
                return header.isEmpty() ? QVariant() : QVariant(header);
            }
            return QVariant();
        }

        QModelIndex CListModelBaseNonTemplate::index(int row, int column, const QModelIndex &parent) const
        {
            Q_UNUSED(parent);
            return QStandardItemModel::createIndex(row, column);
        }

        QModelIndex CListModelBaseNonTemplate::parent(const QModelIndex &child) const
        {
            Q_UNUSED(child);
            return QModelIndex();
        }

        CPropertyIndex CListModelBaseNonTemplate::columnToPropertyIndex(int column) const
        {
            return m_columns.columnToPropertyIndex(column);
        }

        int CListModelBaseNonTemplate::propertyIndexToColumn(const CPropertyIndex &propertyIndex) const
        {
            return m_columns.propertyIndexToColumn(propertyIndex);
        }

        CPropertyIndex CListModelBaseNonTemplate::modelIndexToPropertyIndex(const QModelIndex &index) const
        {
            return this->columnToPropertyIndex(index.column());
        }

        void CListModelBaseNonTemplate::sortByPropertyIndex(const CPropertyIndex &propertyIndex, Qt::SortOrder order)
        {
            const int column = this->propertyIndexToColumn(propertyIndex);
            this->sort(column, order);
        }

        bool CListModelBaseNonTemplate::setSortColumnByPropertyIndex(const CPropertyIndex &propertyIndex)
        {
            const int column = m_columns.propertyIndexToColumn(propertyIndex);
            if (m_sortColumn == column) { return false; } // not changed
            m_sortColumn = column;
            return true; // changed
        }

        bool CListModelBaseNonTemplate::setSorting(const CPropertyIndex &propertyIndex, Qt::SortOrder order)
        {
            const bool changedColumn = this->setSortColumnByPropertyIndex(propertyIndex);
            const bool changedOrder = (m_sortOrder == order);
            m_sortOrder = order;
            return changedColumn || changedOrder;
        }

        bool CListModelBaseNonTemplate::hasValidSortColumn() const
        {

            if (!(m_sortColumn >= 0 && m_sortColumn < m_columns.size())) { return false; }
            return m_columns.isSortable(m_sortColumn);
        }

        Qt::ItemFlags CListModelBaseNonTemplate::flags(const QModelIndex &index) const
        {
            Qt::ItemFlags f = QStandardItemModel::flags(index);
            if (!index.isValid()) { return f; }
            const bool editable = m_columns.isEditable(index);
            f = editable ? (f | Qt::ItemIsEditable) : (f & ~Qt::ItemIsEditable);

            // flags from formatter
            const CDefaultFormatter *formatter = m_columns.getFormatter(index);
            if (formatter) { f = formatter->flags(f, editable); }

            // drag and drop
            f = f | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
            return f;
        }

        const QString &CListModelBaseNonTemplate::getTranslationContext() const
        {
            return m_columns.getTranslationContext();
        }

        Qt::DropActions CListModelBaseNonTemplate::supportedDragActions() const
        {
            return isOrderable() ? Qt::CopyAction | Qt::MoveAction : Qt::CopyAction;
        }

        Qt::DropActions CListModelBaseNonTemplate::supportedDropActions() const
        {
            return m_dropActions;
        }

        QStringList CListModelBaseNonTemplate::mimeTypes() const
        {
            static const QStringList mimes({ "application/swift.container.json" });
            return mimes;
        }

        void CListModelBaseNonTemplate::markDestroyed()
        {
            m_modelDestroyed = true;
        }

        bool CListModelBaseNonTemplate::isModelDestroyed()
        {
            return m_modelDestroyed;
        }

        void CListModelBaseNonTemplate::clearHighlighting()
        {
            // can be overridden to delete highlighting
        }

        bool CListModelBaseNonTemplate::hasHighlightedRows() const
        {
            return false;
            // can be overridden to enable highlighting based operations
        }

        void CListModelBaseNonTemplate::emitDataChanged(int startRowIndex, int endRowIndex)
        {
            BLACK_VERIFY_X(startRowIndex <= endRowIndex, Q_FUNC_INFO, "check rows");
            BLACK_VERIFY_X(startRowIndex >= 0 &&  endRowIndex >= 0, Q_FUNC_INFO, "check rows");

            const int columns = columnCount();
            const int rows = rowCount();
            if (columns < 1) { return; }
            if (startRowIndex < 0) { startRowIndex = 0; }
            if (endRowIndex >= rows) { endRowIndex = rows - 1; }
            const QModelIndex topLeft(createIndex(startRowIndex, 0));
            const QModelIndex bottomRight(createIndex(endRowIndex, columns - 1));
            emit this->dataChanged(topLeft, bottomRight);
        }

        CListModelBaseNonTemplate::CListModelBaseNonTemplate(const QString &translationContext, QObject *parent)
            : QStandardItemModel(parent), m_columns(translationContext), m_sortColumn(-1), m_sortOrder(Qt::AscendingOrder)
        {
            // non unique default name, set translation context as default
            this->setObjectName(translationContext);

            // connect
            connect(this, &CListModelBaseNonTemplate::dataChanged, this, &CListModelBaseNonTemplate::onDataChanged);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        CListModelBase<ObjectType, ContainerType, UseCompare>::CListModelBase(const QString &translationContext, QObject *parent)
            : CListModelBaseNonTemplate(translationContext, parent)
        { }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        int CListModelBase<ObjectType, ContainerType, UseCompare>::rowCount(const QModelIndex &parentIndex) const
        {
            Q_UNUSED(parentIndex);
            return this->containerOrFilteredContainer().size();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
        {
            Q_UNUSED(action);
            Q_UNUSED(row);
            Q_UNUSED(column);
            Q_UNUSED(parent);
            if (!this->isDropAllowed()) { return false; }
            if (!this->acceptDrop(data)) { return false; }
            return true;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
        {
            Q_UNUSED(row);
            Q_UNUSED(column);

            if (!this->isOrderable() || !this->acceptDrop(mimeData)) { return false; }
            const CVariant valueVariant(this->toCVariant(mimeData));
            if (valueVariant.isValid())
            {
                if (action == Qt::MoveAction)
                {
                    const ContainerType container(valueVariant.value<ContainerType>());
                    if (container.isEmpty()) { return false; }
                    const int position = parent.row();
                    this->moveItems(container, position);
                }
            }
            return true;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::isValidIndex(const QModelIndex &index) const
        {
            if (!index.isValid()) { return false; }
            return (index.row() >= 0 && index.row() < this->rowCount(index) &&
                    index.column() >= 0 && index.column() < this->columnCount(index));
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        QVariant CListModelBase<ObjectType, ContainerType, UseCompare>::data(const QModelIndex &index, int role) const
        {
            // check / init
            if (!this->isValidIndex(index)) { return QVariant(); }

            if (role == Qt::BackgroundRole)
            {
                return CListModelBaseNonTemplate::data(index, role);
            }

            // Formatter
            const CDefaultFormatter *formatter = m_columns.getFormatter(index);
            Q_ASSERT_X(formatter, Q_FUNC_INFO, "Missing formatter");

            // Upfront checking avoids unnecessary data fetching
            if (!formatter || !formatter->supportsRole(role)) { return CListModelBaseNonTemplate::data(index, role); }

            // index, updront checking
            const int row = index.row();
            const int col = index.column();
            const CPropertyIndex propertyIndex = this->columnToPropertyIndex(col);
            if (static_cast<int>(CPropertyIndex::GlobalIndexLineNumber) == propertyIndex.frontCasted<int>())
            {
                return QVariant::fromValue(row + 1);
            }

            // Formatted data
            const ObjectType obj = this->containerOrFilteredContainer()[row];
            return formatter->data(role, obj.propertyByIndex(propertyIndex)).getQVariant();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::setData(const QModelIndex &index, const QVariant &value, int role)
        {
            Qt::ItemDataRole dataRole = static_cast<Qt::ItemDataRole>(role);
            if (!(dataRole == Qt::UserRole || dataRole == Qt::EditRole)) { return false; }

            // check / init
            if (!this->isValidIndex(index)) { return false; }
            if (!m_columns.isEditable(index)) { return false; }
            const CDefaultFormatter *formatter = m_columns.getFormatter(index);
            Q_ASSERT(formatter);
            if (!formatter) { return false; }

            ObjectType obj = m_container[index.row()];
            ObjectType currentObject(obj);
            CPropertyIndex propertyIndex = this->columnToPropertyIndex(index.column());
            obj.setPropertyByIndex(propertyIndex, value);

            if (obj != currentObject)
            {
                const QModelIndex topLeft = index.sibling(index.row(), 0);
                const QModelIndex bottomRight = index.sibling(index.row(), this->columnCount() - 1);
                m_container[index.row()] = obj;
                const CVariant co = CVariant::fromValue(obj);
                emit objectChanged(co, propertyIndex);
                emit this->dataChanged(topLeft, bottomRight);
                this->updateFilteredContainer();
                return true;
            }
            return false;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::setInContainer(const QModelIndex &index, const ObjectType &obj)
        {
            if (!index.isValid()) { return false; }
            int row = index.row();
            if (row < 0 || row >= this->container().size()) { return false; }
            m_container[row] = obj;
            return true;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        int CListModelBase<ObjectType, ContainerType, UseCompare>::update(const ContainerType &container, bool sort)
        {
            if (m_modelDestroyed) { return 0; }

            // Keep sorting out of begin/end reset model
            ContainerType sortedContainer;
            ContainerType selection;
            if (m_selectionModel)
            {
                selection = m_selectionModel->selectedObjects();
            }
            const int oldSize = m_container.size();
            const bool performSort = sort && container.size() > 1 && this->hasValidSortColumn();
            if (performSort)
            {
                const int sortColumn = this->getSortColumn();
                sortedContainer = this->sortContainerByColumn(container, sortColumn, m_sortOrder);
            }

            this->beginResetModel();
            m_container = performSort ? sortedContainer : container;
            this->updateFilteredContainer();
            this->endResetModel();

            if (!selection.isEmpty())
            {
                m_selectionModel->selectObjects(selection);
            }

            const int newSize = m_container.size();
            Q_UNUSED(oldSize);
            // I have to update even with same size because I cannot tell what/if data are changed
            this->emitModelDataChanged();
            return newSize;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::update(const QModelIndex &index, const ObjectType &object)
        {
            if (m_modelDestroyed) { return; }
            if (index.row() >= m_container.size()) { return; }
            m_container[index.row()] = object;

            const QModelIndex i1 = index.sibling(index.row(), 0);
            const QModelIndex i2 = index.sibling(index.row(), this->columnCount(index) - 1);
            emit this->dataChanged(i1, i2); // which range has been changed
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::update(int rowIndex, const ObjectType &object)
        {
            this->update(this->index(rowIndex, 0), object);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        CWorker *CListModelBase<ObjectType, ContainerType, UseCompare>::updateAsync(const ContainerType &container, bool sort)
        {
            Q_UNUSED(sort);
            if (m_modelDestroyed) { return nullptr; }
            auto sortColumn = this->getSortColumn();
            auto sortOrder = this->getSortOrder();
            CWorker *worker = CWorker::fromTask(this, "ModelSort", [this, container, sortColumn, sortOrder]()
            {
                return this->sortContainerByColumn(container, sortColumn, sortOrder);
            });
            worker->thenWithResult<ContainerType>(this, [this](const ContainerType & sortedContainer)
            {
                if (m_modelDestroyed) { return;  }
                this->update(sortedContainer, false);
            });
            worker->then(this, &CListModelBase::asyncUpdateFinished);
            return worker;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::updateContainerMaybeAsync(const ContainerType &container, bool sort)
        {
            if (m_modelDestroyed) { return; }
            if (container.size() > asyncThreshold && sort)
            {
                // larger container with sorting
                updateAsync(container, sort);
            }
            else
            {
                update(container, sort);
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::hasFilter() const
        {
            const bool f =  m_filter && m_filter->isValid();
            return f;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::removeFilter()
        {
            if (!this->hasFilter()) { return; }
            m_filter.reset(nullptr);
            this->beginResetModel();
            this->updateFilteredContainer();
            this->endResetModel();
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::takeFilterOwnership(std::unique_ptr<IModelFilter<ContainerType> > &filter)
        {
            if (!filter)
            {
                this->removeFilter(); // clear filter
                return;
            }
            if (filter->isValid())
            {
                m_filter = std::move(filter);
                this->beginResetModel();
                this->updateFilteredContainer();
                this->endResetModel();
                this->emitModelDataChanged();
            }
            else
            {
                // invalid filter, so clear filter
                this->removeFilter();
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        const ObjectType &CListModelBase<ObjectType, ContainerType, UseCompare>::at(const QModelIndex &index) const
        {
            if (index.row() < 0 || index.row() >= this->rowCount())
            {
                static const ObjectType def {}; // default object
                return def;
            }
            else
            {
                return this->containerOrFilteredContainer()[index.row()];
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        const ContainerType &CListModelBase<ObjectType, ContainerType, UseCompare>::container() const
        {
            return m_container;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        const ContainerType &CListModelBase<ObjectType, ContainerType, UseCompare>::containerFiltered() const
        {
            return m_containerFiltered;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        const ContainerType &CListModelBase<ObjectType, ContainerType, UseCompare>::containerOrFilteredContainer(bool *filtered) const
        {
            if (this->hasFilter())
            {
                if (filtered) { *filtered = true; }
                return m_containerFiltered;
            }
            else
            {
                if (filtered) { *filtered = false; }
                return m_container;
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::push_back(const ObjectType &object)
        {
            beginInsertRows(QModelIndex(), m_container.size(), m_container.size());
            m_container.push_back(object);
            endInsertRows();

            if (this->hasFilter())
            {
                // this will change the whole used model as we cannot predict the filter
                this->beginResetModel();
                this->updateFilteredContainer();
                this->endResetModel();
            }
            this->emitModelDataChanged();
        }

        template<typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::push_back(const ContainerType &container)
        {
            beginInsertRows(QModelIndex(), m_container.size(), m_container.size());
            m_container.push_back(container);
            endInsertRows();

            if (this->hasFilter())
            {
                // this will change the whole used model as we cannot predict the filter
                this->beginResetModel();
                this->updateFilteredContainer();
                this->endResetModel();
            }
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::insert(const ObjectType &object)
        {
            beginInsertRows(QModelIndex(), 0, 0);
            m_container.insert(m_container.begin(), object);
            endInsertRows();

            if (this->hasFilter())
            {
                // this will change the whole used model as we cannot predict the filter
                this->beginResetModel();
                this->updateFilteredContainer();
                this->endResetModel();
            }
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::insert(const ContainerType &container)
        {
            if (container.isEmpty()) { return; }
            beginInsertRows(QModelIndex(), 0, 0);
            m_container.insert(container);
            endInsertRows();

            if (this->hasFilter())
            {
                // this will change the whole used model as we cannot predict the filter
                this->beginResetModel();
                this->updateFilteredContainer();
                this->endResetModel();
            }
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::remove(const ObjectType &object)
        {
            int oldSize = m_container.size();
            beginRemoveRows(QModelIndex(), 0, 0);
            m_container.remove(object);
            endRemoveRows();
            int newSize = m_container.size();
            if (oldSize != newSize)
            {
                this->emitModelDataChanged();
                if (this->hasFilter())
                {
                    this->beginResetModel();
                    this->updateFilteredContainer();
                    this->endResetModel();
                }
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::clear()
        {
            beginResetModel();
            m_container.clear();
            m_containerFiltered.clear();
            endResetModel();
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::isEmpty() const
        {
            return m_container.isEmpty();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::updateFilteredContainer()
        {
            if (this->hasFilter())
            {
                m_containerFiltered = m_filter->filter(m_container);
            }
            else
            {
                m_containerFiltered.clear();
            }
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::emitModelDataChanged()
        {
            const int n = this->containerOrFilteredContainer().size();
            emit this->modelDataChanged(n, this->hasFilter());
            emit this->changed();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
        {
            // underlying base class changed
            Q_UNUSED(topLeft);
            Q_UNUSED(bottomRight);
            Q_UNUSED(roles);
            this->emitModelDataChanged();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::onChangedDigest()
        {
            const int n = this->containerOrFilteredContainer().size();
            emit this->changedDigest();
            emit this->modelDataChangedDigest(n, this->hasFilter());
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::moveItems(const ContainerType &items, int position)
        {
            // overridden in specialized class
            Q_UNUSED(items);
            Q_UNUSED(position);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::sort()
        {
            this->sort(this->getSortColumn(), this->getSortOrder());
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::sort(int column, Qt::SortOrder order)
        {
            if (column == m_sortColumn && order == m_sortOrder) { return; }

            // new order
            m_sortColumn = column;
            m_sortOrder  = order;
            if (m_container.size() < 2)
            {
                return; // nothing to do
            }

            // sort the values
            this->updateContainerMaybeAsync(m_container, true);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        void CListModelBase<ObjectType, ContainerType, UseCompare>::truncate(int maxNumber, bool forceSort)
        {
            if (this->rowCount() <= maxNumber) { return; }
            if (forceSort)
            {
                this->sort();    // make sure container is sorted
            }
            ContainerType container(this->container());
            container.truncate(maxNumber);
            this->updateContainerMaybeAsync(container, false);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        ContainerType CListModelBase<ObjectType, ContainerType, UseCompare>::sortContainerByColumn(const ContainerType &container, int column, Qt::SortOrder order) const
        {
            if (m_modelDestroyed) { return container; }
            if (container.size() < 2 || !m_columns.isSortable(column))
            {
                return container;    // nothing to do
            }

            // this is the only part not really thread safe, but columns do not change so far
            const CPropertyIndex propertyIndex = m_columns.columnToSortPropertyIndex(column);
            Q_ASSERT(!propertyIndex.isEmpty());
            if (propertyIndex.isEmpty())
            {
                return container;    // at release build do nothing
            }

            // sort the values
            std::integral_constant<bool, UseCompare> marker {};
            const auto p = [ = ](const ObjectType & a, const ObjectType & b) -> bool
            {
                return Private::compareForModelSort<ObjectType>(a, b, order, propertyIndex, marker);
            };

            return container.sorted(p);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        QMimeData *CListModelBase<ObjectType, ContainerType, UseCompare>::mimeData(const QModelIndexList &indexes) const
        {
            QMimeData *mimeData = new QMimeData();
            if (indexes.isEmpty()) { return mimeData; }

            ContainerType container;
            QList<int> rows; // avoid redundant objects

            // Indexes are per row and column
            for (const QModelIndex &index : indexes)
            {
                if (!index.isValid()) { continue; }
                int r = index.row();
                if (rows.contains(r)) { continue; }
                container.push_back(this->at(index));
                rows.append(r);
            }

            // to JSON via CVariant
            const QJsonDocument containerJson(CVariant::fromValue(container).toJson());
            const QString jsonString(containerJson.toJson(QJsonDocument::Compact));

            mimeData->setData(CGuiUtility::swiftJsonDragAndDropMimeType(), jsonString.toUtf8());
            return mimeData;
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        QJsonObject CListModelBase<ObjectType, ContainerType, UseCompare>::toJson(bool selectedOnly) const
        {
            const CVariant variant = CVariant::fromValue(
                                         selectedOnly && m_selectionModel ?
                                         m_selectionModel->selectedObjects() :
                                         container());
            return variant.toJson();
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        QString CListModelBase<ObjectType, ContainerType, UseCompare>::toJsonString(QJsonDocument::JsonFormat format, bool selectedOnly) const
        {
            const CVariant variant = CVariant::fromValue(
                                         selectedOnly && m_selectionModel ?
                                         m_selectionModel->selectedObjects() :
                                         container());
            return variant.toJsonString(format);
        }

        template <typename ObjectType, typename ContainerType, bool UseCompare>
        bool CListModelBase<ObjectType, ContainerType, UseCompare>::isOrderable() const
        {
            return false;
        }

        // see here for the reason of thess forward instantiations
        // https://isocpp.org/wiki/faq/templates#separate-template-fn-defn-from-decl
        template class CListModelBase<BlackMisc::CIdentifier, BlackMisc::CIdentifierList, false>;
        template class CListModelBase<BlackMisc::CApplicationInfo, BlackMisc::CApplicationInfoList, true>;
        template class CListModelBase<BlackMisc::CStatusMessage, BlackMisc::CStatusMessageList, true>;
        template class CListModelBase<BlackMisc::CNameVariantPair, BlackMisc::CNameVariantPairList, false>;
        template class CListModelBase<BlackMisc::CCountry, BlackMisc::CCountryList, true>;
        template class CListModelBase<BlackMisc::Aviation::CLivery, BlackMisc::Aviation::CLiveryList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAtcStation, BlackMisc::Aviation::CAtcStationList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAirport, BlackMisc::Aviation::CAirportList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAircraftIcaoCode, BlackMisc::Aviation::CAircraftIcaoCodeList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAirlineIcaoCode, BlackMisc::Aviation::CAirlineIcaoCodeList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAircraftParts, BlackMisc::Aviation::CAircraftPartsList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAircraftSituation, BlackMisc::Aviation::CAircraftSituationList, true>;
        template class CListModelBase<BlackMisc::Aviation::CAircraftSituationChange, BlackMisc::Aviation::CAircraftSituationChangeList, true>;
        template class CListModelBase<BlackMisc::Network::CServer, BlackMisc::Network::CServerList, true>;
        template class CListModelBase<BlackMisc::Network::CUser, BlackMisc::Network::CUserList, true>;
        template class CListModelBase<BlackMisc::Network::CTextMessage, BlackMisc::Network::CTextMessageList, false>;
        template class CListModelBase<BlackMisc::Network::CClient, BlackMisc::Network::CClientList, false>;
        template class CListModelBase<BlackMisc::Simulation::CAircraftModel, BlackMisc::Simulation::CAircraftModelList, true>;
        template class CListModelBase<BlackMisc::Simulation::CSimulatedAircraft, BlackMisc::Simulation::CSimulatedAircraftList, true>;
        template class CListModelBase<BlackMisc::Simulation::CDistributor, BlackMisc::Simulation::CDistributorList, true>;
        template class CListModelBase<BlackMisc::Simulation::CInterpolationAndRenderingSetupPerCallsign, BlackMisc::Simulation::CInterpolationSetupList, false>;
        template class CListModelBase<BlackMisc::Simulation::CMatchingStatisticsEntry, BlackMisc::Simulation::CMatchingStatistics, true>;
        template class CListModelBase<BlackMisc::Weather::CCloudLayer, BlackMisc::Weather::CCloudLayerList, false>;
        template class CListModelBase<BlackMisc::Weather::CTemperatureLayer, BlackMisc::Weather::CTemperatureLayerList, false>;
        template class CListModelBase<BlackMisc::Weather::CWindLayer, BlackMisc::Weather::CWindLayerList, false>;

    } // namespace
} // namespace
