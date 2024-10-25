import React, { useState } from 'react';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faSave, faEdit, faTimes } from '@fortawesome/free-solid-svg-icons';

const BinCard = ({ binData, binId, onUpdateBin }) => {
  const [isActive, setIsActive] = useState(binData.status);
  const [binHeight, setBinHeight] = useState(binData.binHeight || '');
  const [isEditing, setIsEditing] = useState(false);
  const [tempBinHeight, setTempBinHeight] = useState(binData.binHeight || '');
  const binFilledPercentage = parseInt(binData.binFilled);

  const getBinFilledColor = () => {
    if (binFilledPercentage < 50) return 'bg-green-500';
    if (binFilledPercentage < 80) return 'bg-yellow-500';
    return 'bg-red-500';
  };

  const handleStatusToggle = async () => {
    const newStatus = !isActive;
    setIsActive(newStatus);
    await onUpdateBin(binId, { status: newStatus });
  };

  const handleEdit = () => {
    setIsEditing(true);
    setTempBinHeight(binData.binHeight || '');
  };

  const handleSave = async () => {
    await onUpdateBin(binId, { binHeight: tempBinHeight });
    setIsEditing(false);
  };

  const handleCancel = () => {
    setIsEditing(false);
    setTempBinHeight(binData.binHeight || '');
  };

  const handleBinHeightChange = (e) => {
    setTempBinHeight(e.target.value);
  };

  return (
    <div className="w-full p-4">
      <div className={`bg-white border ${isActive ? 'border-green-200' : 'border-red-200'} p-6 rounded-xl shadow-lg`}>
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-3xl font-semibold text-gray-800">{binId}</h2>
          <label className="flex items-center cursor-pointer">
            <div className="relative">
              <input type="checkbox" className="sr-only" checked={isActive} onChange={handleStatusToggle} />
              <div className="w-14 h-7 bg-gray-400 rounded-full shadow-inner"></div>
              <div className={`absolute w-9 h-9 rounded-full shadow -left-1 -top-1 transition ${isActive ? 'bg-green-500 transform translate-x-full' : 'bg-red-500'}`}></div>
            </div>
            <div className="ml-3 text-gray-700 font-medium text-lg">
              {isActive ? 'Active' : 'Inactive'}
            </div>
          </label>
        </div>

        <div className="grid grid-cols-2 gap-6 mb-4">
          <div className="text-gray-600 col-span-2">
            <p className="font-medium mb-2">Bin Filled:</p>
            <div className="w-full bg-gray-200 rounded-full h-6 dark:bg-gray-200">
              <div 
                className={`h-6 rounded-full ${getBinFilledColor()} transition-all duration-300 ease-in-out`} 
                style={{width: `${binFilledPercentage}%`}}
              >
                <span className="flex items-center justify-center h-full text-white text-sm font-semibold">
                  {binFilledPercentage}%
                </span>
              </div>
            </div>
          </div>
          <div className="text-gray-600">
            <p className="font-medium">Bin Lid Sensor:</p>
            <p>{binData.binLidSensor}</p>
          </div>
          <div className="text-gray-600">
            <p className="font-medium">Bin Store Sensor:</p>
            <p>{binData.binStoreSensor}</p>
          </div>
          <div className="text-gray-600">
            <p className="font-medium">Lid:</p>
            <p>{binData.lid}</p>
          </div>
          <div className="text-gray-600">
            <p className="font-medium">Bin Height:</p>
            {isEditing ? (
              <div className="flex items-center">
                <input
                  type="number"
                  value={tempBinHeight}
                  onChange={handleBinHeightChange}
                  className="w-20 px-2 py-1 mr-2 border rounded"
                  placeholder="Enter height"
                />
                <button
                  onClick={handleSave}
                  className="p-1 text-green-600 hover:text-green-800"
                  title="Save"
                >
                  <FontAwesomeIcon icon={faSave} />
                </button>
                <button
                  onClick={handleCancel}
                  className="p-1 text-red-600 hover:text-red-800"
                  title="Cancel"
                >
                  <FontAwesomeIcon icon={faTimes} />
                </button>
              </div>
            ) : (
              <div className="flex items-center">
                <p>{binData.binHeight}</p>
                <button
                  onClick={handleEdit}
                  className="ml-2 p-1 text-blue-600 hover:text-blue-800"
                  title="Edit"
                >
                  <FontAwesomeIcon icon={faEdit} />
                </button>
              </div>
            )}
          </div>
        </div>

        {binFilledPercentage >= 95 && (
          <div className="bg-red-100 border-l-4 border-red-500 text-red-700 p-4 mt-4" role="alert">
            <p className="font-bold">Bin is full!</p>
            <p>Take out the trash.</p>
          </div>
        )}
      </div>
    </div>
  );
};

export default BinCard;
