import React, { useState } from 'react';
import { FontAwesomeIcon } from '@fortawesome/react-fontawesome';
import { faSave } from '@fortawesome/free-solid-svg-icons';

const BinCard = ({ binData, binId }) => {
  const [isPowerOn, setIsPowerOn] = useState(binData.status === 'On');
  const [binHeight, setBinHeight] = useState(binData.binHeight || '');
  const binFilledPercentage = parseInt(binData.binFilled);

  const getBinFilledColor = () => {
    if (binFilledPercentage < 50) return 'text-green-500';
    if (binFilledPercentage < 80) return 'text-yellow-500';
    return 'text-red-500';
  };

  const handlePowerToggle = () => {
    setIsPowerOn(!isPowerOn);
  };

  const handleBinHeightChange = (e) => {
    setBinHeight(e.target.value);
  };

  const handleSave = () => {
    // Here you would typically save the binHeight to your backend
    console.log(`Saving bin height for ${binId}: ${binHeight}`);
  };

  return (
    <div className="w-full p-4">
      <div className={`bg-white border ${isPowerOn ? 'border-green-200' : 'border-red-200'} p-6 rounded-xl shadow-lg`}>
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-3xl font-semibold text-gray-800">{binId}</h2>
          <label className="flex items-center cursor-pointer">
            <div className="relative">
              <input type="checkbox" className="sr-only" checked={isPowerOn} onChange={handlePowerToggle} />
              <div className="w-14 h-7 bg-gray-400 rounded-full shadow-inner"></div>
              <div className={`absolute w-9 h-9 rounded-full shadow -left-1 -top-1 transition ${isPowerOn ? 'bg-green-500 transform translate-x-full' : 'bg-red-500'}`}></div>
            </div>
            <div className="ml-3 text-gray-700 font-medium text-lg">
              {isPowerOn ? 'On' : 'Off'}
            </div>
          </label>
        </div>

        <div className="grid grid-cols-3 gap-6 mb-4">
          <div className="text-gray-600">
            <p className="font-medium">Bin Filled:</p>
            <p className={getBinFilledColor()}>{binData.binFilled}</p>
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
            <p className="font-medium">Lid Distance:</p>
            <p>{binData.lidDistance}</p>
          </div>
          <div className="text-gray-600">
            <p className="font-medium">Servo:</p>
            <p>{binData.servo}</p>
          </div>
        </div>

        <div className="mt-4">
          <label htmlFor="binHeight" className="block text-sm font-medium text-gray-700 mb-1">
            Bin Height
          </label>
          <div className="flex rounded-md shadow-sm max-w-xs">
            <input
              type="number"
              name="binHeight"
              id="binHeight"
              className="focus:ring-indigo-500 focus:border-indigo-500 flex-1 block w-full rounded-none rounded-l-md sm:text-sm border-gray-300"
              placeholder="Enter bin height"
              value={binHeight}
              onChange={handleBinHeightChange}
            />
            <button
              type="button"
              className="inline-flex items-center px-3 py-2 border border-transparent text-sm font-medium rounded-r-md text-white bg-indigo-600 hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-indigo-500"
              onClick={handleSave}
            >
              <FontAwesomeIcon icon={faSave} className="mr-2" />
              Save
            </button>
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
