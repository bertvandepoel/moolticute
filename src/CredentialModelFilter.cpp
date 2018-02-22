// Application
#include "CredentialModelFilter.h"
#include "CredentialModel.h"
#include "ServiceItem.h"
#include "LoginItem.h"

CredentialModelFilter::CredentialModelFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
}

CredentialModelFilter::~CredentialModelFilter()
{

}

void CredentialModelFilter::setFilter(const QString &sFilter)
{
    m_sFilter = sFilter;
    invalidateFilter();
}

TreeItem *CredentialModelFilter::getItemByProxyIndex(const QModelIndex &proxyIndex)
{
    QModelIndex srcIndex = mapToSource(proxyIndex);
    CredentialModel *pSrcModel = dynamic_cast<CredentialModel *>(sourceModel());
    return pSrcModel->getItemByIndex(srcIndex);
}

const TreeItem *CredentialModelFilter::getItemByProxyIndex(const QModelIndex &proxyIndex) const
{
    QModelIndex srcIndex = mapToSource(proxyIndex);
    CredentialModel *pSrcModel = dynamic_cast<CredentialModel *>(sourceModel());
    return pSrcModel->getItemByIndex(srcIndex);
}

void CredentialModelFilter::sort(int column, Qt::SortOrder order)
{
    // sort order is not passed as a parameter to lessThan so need
    // to save sort order globally.
    tempSortOrder = order;
    return QSortFilterProxyModel::sort(column, order);
}

bool CredentialModelFilter::filterAcceptsRow(int iSrcRow, const QModelIndex &srcParent) const
{
    // Get source index
    QModelIndex srcIndex = sourceModel()->index(iSrcRow, 0, srcParent);
    if (srcIndex.isValid())
    {
        // If any of children matches the filter, then current index matches the filter as well
        int iRowCount = sourceModel()->rowCount(srcIndex);
        for (int i=0; i<iRowCount; ++i)
            if (filterAcceptsRow(i, srcIndex))
                return true;
        return acceptRow(iSrcRow, srcParent);
    }

    // Parent call for initial behaviour:
    return QSortFilterProxyModel::filterAcceptsRow(iSrcRow, srcParent) ;
}

bool CredentialModelFilter::testItemAgainstNameAndDescription(TreeItem *pItem, const QString &sFilter) const
{
    if (pItem)
    {
        bool bCondition1 = pItem->name().contains(sFilter, Qt::CaseInsensitive);
        bool bCondition2 = pItem->description().contains(sFilter, Qt::CaseInsensitive);
        return bCondition1 || bCondition2;
    }
    return false;
}

bool CredentialModelFilter::acceptRow(int iSrcRow, const QModelIndex &srcParent) const
{
    // Get src model
    CredentialModel *pCredentialModel  = dynamic_cast<CredentialModel *>(sourceModel());

    // Get src index
    QModelIndex srcIndex = pCredentialModel->index(iSrcRow, 0, srcParent);
    if (srcIndex.isValid())
    {
        // Get item
        TreeItem *pItem = pCredentialModel->getItemByIndex(srcIndex);
        if (pItem != nullptr)
        {
            // Is it a login item?
            LoginItem *pLoginItem = dynamic_cast<LoginItem *>(pItem);
            if (pLoginItem != nullptr)
            {
                TreeItem *pParentItem = pLoginItem->parentItem();
                bool bCondition = testItemAgainstNameAndDescription(pParentItem, m_sFilter);
                if (bCondition)
                    return true;
            }

            return testItemAgainstNameAndDescription(pItem, m_sFilter);
        }
    }

    return true;
}

bool CredentialModelFilter::lessThan(const QModelIndex &srcLeft, const QModelIndex &srcRight) const
{
    // Get src model
    CredentialModel *pSrcModel = dynamic_cast<CredentialModel *>(sourceModel());

    TreeItem *pLeftItem = pSrcModel->getItemByIndex(srcLeft);
    TreeItem *pRightItem = pSrcModel->getItemByIndex(srcRight);

    if ((pLeftItem != nullptr) && (pRightItem != nullptr))
    {
        if (srcLeft.column() == 0 && srcRight.column() == 0)
        {
            return pLeftItem->name() < pRightItem->name();
        }

        if ((srcLeft.column() == 1 && srcRight.column() == 1))
        {
            return pLeftItem->bestUpdateDate(tempSortOrder)
                        < pRightItem->bestUpdateDate(tempSortOrder);
        }
    }

    return false;
}

QModelIndex CredentialModelFilter::getProxyIndexFromItem(TreeItem *pItem, int column)
{
    CredentialModel *pCredModel = dynamic_cast<CredentialModel *>(sourceModel());
    switch (pItem->treeType())
    {
    case TreeItem::TreeType::Service:
    {
        QModelIndex serviceIndex = pCredModel->getServiceIndexByName(pItem->name(), column);
        return mapFromSource(serviceIndex);
