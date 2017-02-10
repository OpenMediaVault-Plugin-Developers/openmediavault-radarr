<?php

/**
 * Copyright (C) 2013-2014 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/notify.inc";
require_once "openmediavault/object.inc";
require_once "openmediavault/product.inc";
require_once "openmediavault/rpcservice.inc";
require_once "openmediavault/system.inc";
require_once "openmediavault/util.inc";

class OMVRpcServiceRadarr extends OMVRpcServiceAbstract
{
    private $RadarrSettingsFileA = "/home/radarr/.config/NzbDrone/config.xml";
    private $RadarrSettingsFileB = "/home/radarr/.config/Radarr/config.xml";

    private function getPluginName()
    {
        return strtolower($this->getName());
    }

    private function getEventMessagePath()
    {
        return sprintf("org.openmediavault.service.%s", $this->getPluginName());
    }

    private function getXPath()
    {
        return sprintf("/config/services/%s", $this->getPluginName());
    }

    private function getXPathForks()
    {
        return sprintf("%s/forks", $this->getXPath());
    }

    private function getXPathFork()
    {
        return sprintf("%s/fork", $this->getXPathForks());
    }

    public function getName()
    {
        return "Radarr";
    }

    public function initialize()
    {
        $this->registerMethod("getSettings");
        $this->registerMethod("setSettings");
    }

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $port = 7878;

        // Get current port as set in Radarr's config file
        if (file_exists($this->RadarrSettingsFileA)) {
            $config = simplexml_load_file(
                $this->RadarrSettingsFileA
            );

            if (!empty($config->Port)) {
                $port = $config->Port;
            }
        }elseif (file_exists($this->RadarrSettingsFileB)) {
            $config = simplexml_load_file(
                $this->RadarrSettingsFileB
            );

            if (!empty($config->Port)) {
                $port = $config->Port;
            }
        }

        $object["enable"]  = boolval($object["enable"]);
        $object["showtab"] = boolval($object["showtab"]);
        $object["port"]    = intval($port);

        return $object;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        $this->validateMethodContext($context, array(
            "role" => OMV_ROLE_ADMINISTRATOR
        ));

        $this->validateMethodParams(
            $params,
            '{
                "type"       : "object",
                "properties" : {
                    "enable"    : { "type" : "boolean" },
                    "showtab"   : { "type" : "boolean" },
                    "port"      : { "type" : "integer" }
                }
            }'
        );

        $object = array(
            "enable"    => array_boolval($params, "enable"),
            "showtab"   => array_boolval($params, "showtab"),
            "port"      => $params["port"],
        );

        if (false === $xmlConfig->replace($this->getXPath(), $object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED,
                $this->getXPath()
            );
        }

        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            $object
        );

        return $object;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRadarr());
