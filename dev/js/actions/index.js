export const selectProbe = (probe) => {
    console.log("You clicked on Probe ID: ", probe.id);
    return {
        type: 'PROBE_SELECTED',
        payload: probe
    }
};

