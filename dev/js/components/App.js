import React from 'react';
import ProbeList from '../containers/probe-list';
import ProbeDetails from '../containers/probe-detail';

require('../../scss/style.scss');


const App = () => (
    <div>
        <h2>Probe List</h2>
        <ProbeList />
        <hr />
        <h2>Probe Details</h2>
        <ProbeDetails />
    </div>
);

export default App;