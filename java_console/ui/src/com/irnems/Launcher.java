package com.irnems;

import com.irnems.core.EngineState;
import com.irnems.core.MessagesCentral;
import com.rusefi.*;
import com.rusefi.io.LinkManager;
import com.rusefi.ui.*;
import jssc.SerialPortList;

import javax.swing.*;

/**
 * this is the main entry point of rusEfi ECU console
 * <p/>
 * <p/>
 * Date: 12/25/12
 * (c) Andrey Belomutskiy
 *
 * @see WavePanel
 */
public class Launcher extends FrameHelper {
    public static final int CONSOLE_VERSION = 20140908;
    public static final boolean SHOW_STIMULATOR = true;
    private final String port;

    public Launcher(String port) {
        this.port = port;
        FileLog.MAIN.start();

        LinkManager.start(port);

        FileLog.MAIN.logLine("Console " + CONSOLE_VERSION);

        JTabbedPane tabbedPane = new JTabbedPane();

        RpmPanel rpmPanel = new RpmPanel();
        tabbedPane.addTab("Main", rpmPanel.createRpmPanel());
        tabbedPane.addTab("Gauges", new GaugesPanel().getContent());
        tabbedPane.addTab("Digital Sniffer", WavePanel.getInstance().getPanel());
        tabbedPane.addTab("Analog Sniffer", new AnalogChartPanel());

//        tabbedPane.addTab("ADC", new AdcPanel(new BooleanInputsModel()).createAdcPanel());
        if (SHOW_STIMULATOR) {
            EcuStimulator stimulator = EcuStimulator.getInstance();
            tabbedPane.add("ECU stimulation", stimulator.getPanel());
        }
//        tabbedPane.addTab("live map adjustment", new Live3DReport().getControl());
        tabbedPane.add("Messages", new MsgPanel(true).getContent());

        tabbedPane.add("Log Viewer", new LogViewer());

        tabbedPane.setSelectedIndex(2);
//        tabbedPane.setSelectedIndex(5);

        showFrame(tabbedPane);
    }

    @Override
    protected void onWindowOpened() {
        super.onWindowOpened();
        setTitle("N/A");

        LinkManager.open();

        LinkManager.engineState.registerStringValueAction("rusEfiVersion", new EngineState.ValueCallback<String>() {
            @Override
            public void onUpdate(String firmwareVersion) {
                setTitle(firmwareVersion);
                VersionChecker.getInstance().onFirmwareVersion(firmwareVersion);
            }
        });
    }

    private void setTitle(String value) {
        frame.setTitle("Console " + CONSOLE_VERSION + "; firmware=" + value + "@" + port);
    }

    @Override
    protected void onWindowClosed() {
        super.onWindowClosed();
        /**
         * looks like reconnectTimer in {@link RpmPanel} keeps AWT alive. Simplest solution would be to 'exit'
         */
        SimulatorHelper.onWindowClosed();
        System.exit(0);
    }

    public static void main(final String[] args) throws Exception {
        Thread.setDefaultUncaughtExceptionHandler(new DefaultExceptionHandler());
        VersionChecker.start();
        SwingUtilities.invokeAndWait(new Runnable() {
            public void run() {
                awtCode(args);
            }
        });
    }

    private static void awtCode(String[] args) {
        try {

            boolean isPortDefined = args.length > 0;
            if (isPortDefined) {
                new Launcher(args[0]);
            } else {
                for (String p : SerialPortList.getPortNames())
                    MessagesCentral.getInstance().postMessage(Launcher.class, "Available port: " + p);
                PortLookupFrame.chooseSerialPort();
            }

        } catch (Throwable e) {
            throw new IllegalStateException(e);
        }
    }
}
